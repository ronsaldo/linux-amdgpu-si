/*
 * Copyright 2012 Advanced Micro Devices, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE COPYRIGHT HOLDER(S) OR AUTHOR(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 */
#include "drmP.h"
#include "amdgpu.h"
#include "amdgpu_ih.h"
#include "sid.h"
#include "si_reg.h"

#include "dce/dce_6_0_d.h"
#include "dce/dce_6_0_sh_mask.h"
#include "dce/dce_6_0_enum.h"

#include "bif/bif_4_0_d.h"
#include "bif/bif_4_0_enum.h"
#include "bif/bif_4_0_sh_mask.h"

#include "oss/oss_1_0_d.h"
#include "oss/oss_1_0_enum.h"
#include "oss/oss_1_0_sh_mask.h"

#include "gca/gfx_6_0_d.h"
#include "gca/gfx_6_0_enum.h"
#include "gca/gfx_6_0_sh_mask.h"

#include "gmc/gmc_6_0_d.h"
#include "gmc/gmc_6_0_enum.h"
#include "gmc/gmc_6_0_sh_mask.h"

static const u32 dma_offsets[DMA_MAX_INSTANCE] =
{
	DMA0_REGISTER_OFFSET,
	DMA1_REGISTER_OFFSET
};

static void si_dma_set_ring_funcs(struct amdgpu_device *adev);
static void si_dma_set_irq_funcs(struct amdgpu_device *adev);
static void si_dma_set_buffer_funcs(struct amdgpu_device *adev);
static void si_dma_set_vm_pte_funcs(struct amdgpu_device *adev);

/*
 * DMA
 * Starting with R600, the GPU has an asynchronous
 * DMA engine.  The programming model is very similar
 * to the 3D engine (ring buffer, IBs, etc.), but the
 * DMA controller has it's own packet format that is
 * different form the PM4 format used by the 3D engine.
 * It supports copying data, writing embedded data,
 * solid fills, and a number of other things.  It also
 * has support for tiling/detiling of buffers.
 * Cayman and newer support two asynchronous DMA engines.
 */

/**
 * cayman_dma_get_rptr - get the current read pointer
 *
 * @adev: amdgpu_device pointer
 * @ring: radeon ring pointer
 *
 * Get the current rptr from the hardware (cayman+).
 */
 static uint32_t si_dma_ring_get_rptr(struct amdgpu_ring *ring)
 {
 	u32 rptr;

 	rptr = ring->adev->wb.wb[ring->rptr_offs];

 	return (rptr & 0x3fffc) >> 2;
 }

 /**
  * cayman_dma_get_wptr - get the current write pointer
  *
  * @adev: amdgpu_device pointer
  * @ring: radeon ring pointer
  *
  * Get the current wptr from the hardware (cayman+).
  */
 static uint32_t si_dma_ring_get_wptr(struct amdgpu_ring *ring)
 {
    struct amdgpu_device *adev = ring->adev;
    u32 me = (ring == &adev->sdma.instance[0].ring) ? 0 : 1;

 	return (RREG32(mmDMA_RB_WPTR + dma_offsets[me]) & 0x3fffc) >> 2;
 }

 /**
  * cayman_dma_set_wptr - commit the write pointer
  *
  * @adev: amdgpu_device pointer
  * @ring: radeon ring pointer
  *
  * Write the wptr back to the hardware (cayman+).
  */
 void si_dma_ring_set_wptr(struct amdgpu_ring *ring)
 {
     struct amdgpu_device *adev = ring->adev;
     u32 me = (ring == &adev->sdma.instance[0].ring) ? 0 : 1;

     WREG32(mmDMA_RB_WPTR + dma_offsets[me], (ring->wptr << 2) & 0x3fffc);
 }

 static void si_dma_ring_insert_nop(struct amdgpu_ring *ring, uint32_t count)
 {
 	int i;

 	for (i = 0; i < count; i++)
 		amdgpu_ring_write(ring, ring->nop);
 }

 /**
  * cayman_dma_ring_ib_execute - Schedule an IB on the DMA engine
  *
  * @adev: amdgpu_device pointer
  * @ib: IB object to schedule
  *
  * Schedule an IB in the DMA ring (cayman-SI).
  */
 void si_dma_ring_emit_ib(struct amdgpu_ring *ring,
 			   struct amdgpu_ib *ib)
 {
 	unsigned vm_id = ib->vm ? ib->vm->ids[ring->idx].id : 0;

	u32 next_rptr = ring->wptr + 4;
	while ((next_rptr & 7) != 5)
		next_rptr++;

	next_rptr += 3;
	amdgpu_ring_write(ring, DMA_PACKET(DMA_PACKET_WRITE, 0, 0, 1));
	amdgpu_ring_write(ring, ring->next_rptr_gpu_addr & 0xfffffffc);
	amdgpu_ring_write(ring, upper_32_bits(ring->next_rptr_gpu_addr) & 0xff);
	amdgpu_ring_write(ring, next_rptr);

 	/* The indirect buffer packet must end on an 8 DW boundary in the DMA ring.
 	 * Pad as necessary with NOPs.
 	 */
     /* IB packet must end on a 8 DW boundary */
 	si_dma_ring_insert_nop(ring, (12 - (ring->wptr & 7)) % 8);

 	amdgpu_ring_write(ring, DMA_IB_PACKET(DMA_PACKET_INDIRECT_BUFFER, vm_id, 0));
 	amdgpu_ring_write(ring, (ib->gpu_addr & 0xFFFFFFE0));
 	amdgpu_ring_write(ring, (ib->length_dw << 12) | (upper_32_bits(ib->gpu_addr) & 0xFF));
 }

 /**
  * si_dma_ring_emit_hdp_flush - emit an hdp flush on the DMA ring
  *
  * @ring: amdgpu ring pointer
  *
  * Emit an hdp flush packet on the requested DMA ring.
  */
 static void si_dma_ring_emit_hdp_flush(struct amdgpu_ring *ring)
 {
	amdgpu_ring_write(ring, DMA_PACKET(DMA_PACKET_SRBM_WRITE, 0, 0, 0));
	amdgpu_ring_write(ring, (0xf << 16) | (mmHDP_MEM_COHERENCY_FLUSH_CNTL));
	amdgpu_ring_write(ring, 1);
 }

 /**
  * si_dma_fence_ring_emit - emit a fence on the DMA ring
  *
  * @adev: amdgpu_device pointer
  * @fence: radeon fence object
  *
  * Add a DMA fence packet to the ring to write
  * the fence seq number and DMA trap packet to generate
  * an interrupt if needed (evergreen-SI).
  */
 void si_dma_ring_emit_fence(struct amdgpu_ring *ring, u64 addr, u64 seq,
 				     unsigned flags)
 {
    bool write64bit = flags & AMDGPU_FENCE_FLAG_64BIT;
    printk(KERN_ALERT "emit fence 0x%08X %d\n", (u32)seq, (int)write64bit);

 	/* write the fence */
 	amdgpu_ring_write(ring, DMA_PACKET(DMA_PACKET_FENCE, 0, 0, 0));
    amdgpu_ring_write(ring, (lower_32_bits(addr) & 0xfffffffc));
	amdgpu_ring_write(ring, (upper_32_bits(addr) & 0xff));
	amdgpu_ring_write(ring, lower_32_bits(seq));
    if(write64bit) {
        addr += 4;
        amdgpu_ring_write(ring, DMA_PACKET(DMA_PACKET_FENCE, 0, 0, 0));
        amdgpu_ring_write(ring, (lower_32_bits(addr) & 0xfffffffc));
    	amdgpu_ring_write(ring, (upper_32_bits(addr) & 0xff));
    	amdgpu_ring_write(ring, upper_32_bits(seq));
    }

 	/* generate an interrupt */
 	amdgpu_ring_write(ring, DMA_PACKET(DMA_PACKET_TRAP, 0, 0, 0));

    si_dma_ring_emit_hdp_flush(ring);

    printk(KERN_ALERT "emit fence end\n", (u32)seq);
 }

 /**
 * r600_dma_semaphore_ring_emit - emit a semaphore on the dma ring
 *
 * @adev: amdgpu_device pointer
 * @ring: amdgpu_ring structure holding ring information
 * @semaphore: radeon semaphore object
 * @emit_wait: wait or signal semaphore
 *
 * Add a DMA semaphore packet to the ring wait on or signal
 * other rings (r6xx-SI).
 */
bool si_dma_ring_emit_semaphore(struct amdgpu_ring *ring,
					 struct amdgpu_semaphore *semaphore,
					 bool emit_wait)
{
	u64 addr = semaphore->gpu_addr;
	u32 s = emit_wait ? 0 : 1;

	amdgpu_ring_write(ring, DMA_PACKET(DMA_PACKET_SEMAPHORE, 0, s, 0));
	amdgpu_ring_write(ring, addr & 0xfffffffc);
	amdgpu_ring_write(ring, upper_32_bits(addr) & 0xff);

	return true;
}

/**
 * cayman_dma_stop - stop the async dma engines
 *
 * @adev: amdgpu_device pointer
 *
 * Stop the async dma engines (cayman-SI).
 */
void si_dma_stop(struct amdgpu_device *adev)
{
    struct amdgpu_ring *sdma0 = &adev->sdma.instance[0].ring;
	struct amdgpu_ring *sdma1 = &adev->sdma.instance[1].ring;
	u32 rb_cntl;
	int i;

	if ((adev->mman.buffer_funcs_ring == sdma0) ||
	    (adev->mman.buffer_funcs_ring == sdma1))
		amdgpu_ttm_set_active_vram_size(adev, adev->mc.visible_vram_size);

	/* dma0 */
	rb_cntl = RREG32(mmDMA_RB_CNTL + DMA0_REGISTER_OFFSET);
	rb_cntl &= ~DMA_RB_CNTL__DMA_RB_ENABLE_MASK;
	WREG32(mmDMA_RB_CNTL + DMA0_REGISTER_OFFSET, rb_cntl);

	/* dma1 */
	rb_cntl = RREG32(mmDMA_RB_CNTL + DMA1_REGISTER_OFFSET);
	rb_cntl &= ~DMA_RB_CNTL__DMA_RB_ENABLE_MASK;
	WREG32(mmDMA_RB_CNTL + DMA1_REGISTER_OFFSET, rb_cntl);

    sdma0->ready = false;
	sdma1->ready = false;
}

/**
 * cik_dma_enable - stop the async dma engines
 *
 * @adev: amdgpu_device pointer
 * @enable: enable/disable the DMA MEs.
 *
 * Halt or unhalt the async dma engines (CIK).
 */
static void si_dma_enable(struct amdgpu_device *adev, bool enable)
{
	u32 me_cntl;
	int i;

	if (enable == false) {
		si_dma_stop(adev);
	}
}

/**
 * cayman_dma_resume - setup and start the async dma engines
 *
 * @adev: amdgpu_device pointer
 *
 * Set up the DMA ring buffers and enable them. (cayman-SI).
 * Returns 0 for success, error for failure.
 */
int si_dma_gfx_resume(struct amdgpu_device *adev)
{
	struct amdgpu_ring *ring;
	u32 rb_cntl, dma_cntl, ib_cntl;
	u32 rb_bufsz;
	u32 reg_offset, wb_offset;
	int i, r;

	for (i = 0; i < 2; i++) {
        ring = &adev->sdma.instance[i].ring;
        wb_offset = (ring->rptr_offs * 4);
		if (i == 0) {
			reg_offset = DMA0_REGISTER_OFFSET;
		} else {
			reg_offset = DMA1_REGISTER_OFFSET;
		}

		WREG32(mmDMA_SEM_INCOMPLETE_TIMER_CNTL + reg_offset, 0);
		WREG32(mmDMA_SEM_WAIT_FAIL_TIMER_CNTL + reg_offset, 0);

		/* Set ring buffer size in dwords */
		rb_bufsz = order_base_2(ring->ring_size / 4);
		rb_cntl = rb_bufsz << 1;
#ifdef __BIG_ENDIAN
		rb_cntl |= DMA_RB_CNTL__DMA_RB_SWAP_ENABLE_MASK | DMA_RB_CNTL__DMA_RPTR_WRITEBACK_SWAP_ENABLE_MASK;
#endif
		WREG32(mmDMA_RB_CNTL + reg_offset, rb_cntl);

		/* Initialize the ring buffer's read and write pointers */
		WREG32(mmDMA_RB_RPTR + reg_offset, 0);
		WREG32(mmDMA_RB_WPTR + reg_offset, 0);

		/* set the wb address whether it's enabled or not */
		WREG32(mmDMA_RB_RPTR_ADDR_HI + reg_offset,
		       upper_32_bits(adev->wb.gpu_addr + wb_offset) & 0xFF);
		WREG32(mmDMA_RB_RPTR_ADDR_LO + reg_offset,
		       ((adev->wb.gpu_addr + wb_offset) & 0xFFFFFFFC));

		rb_cntl |= DMA_RB_CNTL__DMA_RPTR_WRITEBACK_ENABLE_MASK;

		WREG32(mmDMA_RB_BASE + reg_offset, ring->gpu_addr >> 8);

		/* enable DMA IBs */
		ib_cntl = DMA_IB_CNTL__DMA_IB_ENABLE_MASK | DMA_IB_CNTL__CMD_VMID_FORCE_MASK;
#ifdef __BIG_ENDIAN
		ib_cntl |= DMA_IB_CNTL__DMA_IB_SWAP_ENABLE_MASK;
#endif
		WREG32(mmDMA_IB_CNTL + reg_offset, ib_cntl);

		dma_cntl = RREG32(mmDMA_CNTL + reg_offset);
		dma_cntl &= ~DMA_CNTL__CTXEMPTY_INT_ENABLE_MASK;
		WREG32(mmDMA_CNTL + reg_offset, dma_cntl);

		ring->wptr = 0;
		WREG32(mmDMA_RB_WPTR + reg_offset, ring->wptr << 2);

		WREG32(mmDMA_RB_CNTL + reg_offset, rb_cntl | DMA_RB_CNTL__DMA_RB_ENABLE_MASK);

		ring->ready = true;

        /* DEBUG */printk(KERN_ALERT "Call test_ring\n");
		r = amdgpu_ring_test_ring(ring);
        /* DEBUG */printk(KERN_ALERT "Call test_ring done\n");
		if (r) {
			ring->ready = false;
			return r;
		}
	}

	if (adev->mman.buffer_funcs_ring == ring)
		amdgpu_ttm_set_active_vram_size(adev, adev->mc.real_vram_size);

	return 0;
}

/**
 * si_dma_start - setup and start the async dma engines
 *
 * @adev: amdgpu_device pointer
 *
 * Set up the DMA engines and enable them (CIK).
 * Returns 0 for success, error for failure.
 */
static int si_dma_start(struct amdgpu_device *adev)
{
	int r;

	/* unhalt the MEs */
    /* DEBUG */ printk(KERN_ALERT "si_dma_start si_dma enable");
	si_dma_enable(adev, true);

	/* start the gfx rings and rlc compute queues */
    /* DEBUG */ printk(KERN_ALERT "si_dma_start si_dma_gfx_resume");
	r = si_dma_gfx_resume(adev);
	if (r)
		return r;

	return 0;
}

/**
 * r600_dma_ring_test - simple async dma engine test
 *
 * @adev: amdgpu_device pointer
 * @ring: amdgpu_ring structure holding ring information
 *
 * Test the DMA engine by writing using it to write an
 * value to memory. (r6xx-SI).
 * Returns 0 for success, error for failure.
 */
static int si_dma_ring_test_ring(struct amdgpu_ring *ring)
{
    struct amdgpu_device *adev = ring->adev;
	unsigned i;
	int r;
	unsigned index;
	u32 tmp;
	u64 gpu_addr;

    r = amdgpu_wb_get(adev, &index);
	if (r) {
		dev_err(adev->dev, "(%d) failed to allocate wb slot\n", r);
		return r;
	}

	gpu_addr = adev->wb.gpu_addr + index*4;

	tmp = 0xCAFEDEAD;
	adev->wb.wb[index] = cpu_to_le32(tmp);

	r = amdgpu_ring_lock(ring, 4);
	if (r) {
		DRM_ERROR("amdgpu: dma failed to lock ring %d (%d).\n", ring->idx, r);
        amdgpu_wb_free(adev, index);
		return r;
	}
	amdgpu_ring_write(ring, DMA_PACKET(DMA_PACKET_WRITE, 0, 0, 1));
	amdgpu_ring_write(ring, lower_32_bits(gpu_addr));
	amdgpu_ring_write(ring, upper_32_bits(gpu_addr) & 0xff);
	amdgpu_ring_write(ring, 0xDEADBEEF);
	amdgpu_ring_unlock_commit(ring);

	for (i = 0; i < adev->usec_timeout; i++) {
		tmp = le32_to_cpu(adev->wb.wb[index]);
		if (tmp == 0xDEADBEEF)
			break;
		DRM_UDELAY(1);
	}

	if (i < adev->usec_timeout) {
		DRM_INFO("ring test on %d succeeded in %d usecs\n", ring->idx, i);
	} else {
		DRM_ERROR("amdgpu: ring %d test failed (0x%08X)\n",
			  ring->idx, tmp);
		r = -EINVAL;
	}
    amdgpu_wb_free(adev, index);

	return r;
}

/**
 * r600_dma_ib_test - test an IB on the DMA engine
 *
 * @adev: amdgpu_device pointer
 * @ring: amdgpu_ring structure holding ring information
 *
 * Test a simple IB in the DMA ring (r6xx-SI).
 * Returns 0 on success, error on failure.
 */
static int si_dma_ring_test_ib(struct amdgpu_ring *ring)
{
    struct amdgpu_device *adev = ring->adev;
	struct amdgpu_ib ib;
    struct fence *f = NULL;
	unsigned i;
	unsigned index;
	int r;
	u32 tmp = 0;
	u64 gpu_addr;

    printk(KERN_ALERT "si_dma_ring_test_ib start\n");
    r = amdgpu_wb_get(adev, &index);
	if (r) {
		dev_err(adev->dev, "(%d) failed to allocate wb slot\n", r);
		return r;
	}

	/* DEBUG */
	r = amdgpu_ring_lock(ring, 1);
	if (r) {
		DRM_ERROR("amdgpu: dma failed to lock ring %d (%d).\n", ring->idx, r);
        amdgpu_wb_free(adev, index);
		return r;
	}
	amdgpu_ring_write(ring, DMA_PACKET(DMA_PACKET_TRAP, 0, 0, 0));
	amdgpu_ring_unlock_commit(ring);
	/* DEBUG */

	gpu_addr = adev->wb.gpu_addr + (index * 4);
    tmp = 0xCAFEDEAD;
	adev->wb.wb[index] = cpu_to_le32(tmp);
    memset(&ib, 0, sizeof(ib));
	r = amdgpu_ib_get(ring, NULL, 256, &ib);
	if (r) {
		DRM_ERROR("amdgpu: failed to get ib (%d).\n", r);
		goto err0;
	}

	ib.ptr[0] = DMA_PACKET(DMA_PACKET_WRITE, 0, 0, 1);
	ib.ptr[1] = lower_32_bits(gpu_addr);
	ib.ptr[2] = upper_32_bits(gpu_addr) & 0xff;
	ib.ptr[3] = 0xDEADBEEF;
	ib.length_dw = 4;

	r = amdgpu_sched_ib_submit_kernel_helper(adev, ring, &ib, 1, NULL,
						 AMDGPU_FENCE_OWNER_UNDEFINED,
						 &f);
	if (r) {
		DRM_ERROR("amdgpu: failed to schedule ib (%d).\n", r);
		goto err1;
	}
    printk(KERN_ALERT "si_dma_ring_test_ib wait_fence\n");
	r = fence_wait(f, false);
	if (r) {
		DRM_ERROR("amdgpu: fence wait failed (%d).\n", r);
		goto err1;
	}
	for (i = 0; i < adev->usec_timeout; i++) {
		tmp = le32_to_cpu(adev->wb.wb[index/4]);
		if (tmp == 0xDEADBEEF)
			break;
		DRM_UDELAY(1);
	}
	if (i < adev->usec_timeout) {
		DRM_INFO("ib test on ring %d succeeded in %u usecs\n", ring->idx, i);
	} else {
		DRM_ERROR("amdgpu: ib test failed (0x%08X)\n", tmp);
		r = -EINVAL;
	}

err1:
	fence_put(f);
	amdgpu_ib_free(adev, &ib);
err0:
	amdgpu_wb_free(adev, index);
    printk(KERN_ALERT "si_dma_ring_test_ib end\n");
	return r;
}

/**
 * si_dma_vm_copy_pages - update PTEs by copying them from the GART
 *
 * @rdev: radeon_device pointer
 * @ib: indirect buffer to fill with commands
 * @pe: addr of the page entry
 * @src: src addr where to copy from
 * @count: number of page entries to update
 *
 * Update PTEs by copying them from the GART using the DMA (SI).
 */
void si_dma_vm_copy_pte(struct amdgpu_ib *ib,
			  uint64_t pe, uint64_t src,
			  unsigned count)
{
	while (count) {
		unsigned bytes = count * 8;
		if (bytes > 0xFFFF8)
			bytes = 0xFFFF8;

		ib->ptr[ib->length_dw++] = SI_DMA_PACKET(DMA_PACKET_COPY,
						      1, 0, 0, bytes);
		ib->ptr[ib->length_dw++] = lower_32_bits(pe);
		ib->ptr[ib->length_dw++] = lower_32_bits(src);
		ib->ptr[ib->length_dw++] = upper_32_bits(pe) & 0xff;
		ib->ptr[ib->length_dw++] = upper_32_bits(src) & 0xff;

		pe += bytes;
		src += bytes;
		count -= bytes / 8;
	}
}

/**
 * si_dma_vm_write_pages - update PTEs by writing them manually
 *
 * @rdev: radeon_device pointer
 * @ib: indirect buffer to fill with commands
 * @pe: addr of the page entry
 * @addr: dst addr to write into pe
 * @count: number of page entries to update
 * @incr: increase next addr by incr bytes
 * @flags: access flags
 *
 * Update PTEs by writing them manually using the DMA (SI).
 */
void si_dma_vm_write_pte(struct amdgpu_ib *ib,
			   uint64_t pe,
			   uint64_t addr, unsigned count,
			   uint32_t incr, uint32_t flags)
{
    struct amdgpu_device *adev = ib->ring->adev;
	uint64_t value;
	unsigned ndw;

	while (count) {
		ndw = count * 2;
		if (ndw > 0xFFFFE)
			ndw = 0xFFFFE;

		/* for non-physically contiguous pages (system) */
		ib->ptr[ib->length_dw++] = SI_DMA_PACKET(DMA_PACKET_WRITE, 0, 0, 0, ndw);
		ib->ptr[ib->length_dw++] = pe;
		ib->ptr[ib->length_dw++] = upper_32_bits(pe) & 0xff;
		for (; ndw > 0; ndw -= 2, --count, pe += 8) {
			if (flags & AMDGPU_PTE_SYSTEM) {
				value = amdgpu_vm_map_gart(adev, addr);
			} else if (flags & AMDGPU_PTE_VALID) {
				value = addr;
			} else {
				value = 0;
			}
			addr += incr;
			value |= flags;
			ib->ptr[ib->length_dw++] = value;
			ib->ptr[ib->length_dw++] = upper_32_bits(value);
		}
	}
}

/**
 * si_dma_vm_set_pages - update the page tables using the DMA
 *
 * @rdev: radeon_device pointer
 * @ib: indirect buffer to fill with commands
 * @pe: addr of the page entry
 * @addr: dst addr to write into pe
 * @count: number of page entries to update
 * @incr: increase next addr by incr bytes
 * @flags: access flags
 *
 * Update the page tables using the DMA (SI).
 */
void si_dma_vm_set_pte_pde(struct amdgpu_ib *ib,
			 uint64_t pe,
			 uint64_t addr, unsigned count,
			 uint32_t incr, uint32_t flags)
{
	uint64_t value;
	unsigned ndw;

	while (count) {
		ndw = count * 2;
		if (ndw > 0xFFFFE)
			ndw = 0xFFFFE;

		if (flags & AMDGPU_PTE_VALID)
			value = addr;
		else
			value = 0;

		/* for physically contiguous pages (vram) */
		ib->ptr[ib->length_dw++] = DMA_PTE_PDE_PACKET(ndw);
		ib->ptr[ib->length_dw++] = pe; /* dst addr */
		ib->ptr[ib->length_dw++] = upper_32_bits(pe) & 0xff;
		ib->ptr[ib->length_dw++] = flags; /* mask */
		ib->ptr[ib->length_dw++] = 0;
		ib->ptr[ib->length_dw++] = value; /* value */
		ib->ptr[ib->length_dw++] = upper_32_bits(value);
		ib->ptr[ib->length_dw++] = incr; /* increment size */
		ib->ptr[ib->length_dw++] = 0;
		pe += ndw * 4;
		addr += (ndw / 2) * incr;
		count -= ndw / 2;
	}
}

/**
 * si_dma_vm_pad_ib - pad the IB to the required number of dw
 *
 * @ib: indirect buffer to fill with padding
 *
 */
static void si_dma_vm_pad_ib(struct amdgpu_ib *ib)
{
	u32 pad_count;
	int i;

	pad_count = (8 - (ib->length_dw & 0x7)) % 8;
	for (i = 0; i < pad_count; i++)
		ib->ptr[ib->length_dw++] = DMA_PACKET(DMA_PACKET_NOP, 0, 0, 0);
}

static void si_dma_ring_emit_vm_flush(struct amdgpu_ring *ring,
					unsigned vm_id, uint64_t pd_addr)

{
	amdgpu_ring_write(ring, SI_DMA_PACKET(DMA_PACKET_SRBM_WRITE, 0, 0, 0, 0));
	if (vm_id < 8) {
		amdgpu_ring_write(ring, (0xf << 16) | (mmVM_CONTEXT0_PAGE_TABLE_BASE_ADDR + vm_id));
	} else {
		amdgpu_ring_write(ring, (0xf << 16) | (mmVM_CONTEXT8_PAGE_TABLE_BASE_ADDR + (vm_id - 8)));
	}
	amdgpu_ring_write(ring, pd_addr >> 12);

	/* flush hdp cache */
	amdgpu_ring_write(ring, SI_DMA_PACKET(DMA_PACKET_SRBM_WRITE, 0, 0, 0, 0));
	amdgpu_ring_write(ring, (0xf << 16) | (mmHDP_MEM_COHERENCY_FLUSH_CNTL));
	amdgpu_ring_write(ring, 1);

	/* bits 0-7 are the VM contexts0-7 */
	amdgpu_ring_write(ring, SI_DMA_PACKET(DMA_PACKET_SRBM_WRITE, 0, 0, 0, 0));
	amdgpu_ring_write(ring, (0xf << 16) | (mmVM_INVALIDATE_REQUEST));
	amdgpu_ring_write(ring, 1 << vm_id);

	/* wait for invalidate to complete */
	amdgpu_ring_write(ring, SI_DMA_PACKET(DMA_PACKET_POLL_REG_MEM, 0, 0, 0, 0));
	amdgpu_ring_write(ring, mmVM_INVALIDATE_REQUEST << 2);
	amdgpu_ring_write(ring, 0xff << 16); /* retry */
	amdgpu_ring_write(ring, 1 << vm_id); /* mask */
	amdgpu_ring_write(ring, 0); /* value */
	amdgpu_ring_write(ring, (0 << 28) | 0x20); /* func(always) | poll interval */
}

static void si_enable_dma_mgcg(struct amdgpu_device *adev,
			       bool enable)
{
	u32 orig, data, offset;
	int i;

	if (enable && (adev->cg_flags & AMDGPU_CG_SUPPORT_SDMA_MGCG)) {
		for (i = 0; i < 2; i++) {
			if (i == 0)
				offset = DMA0_REGISTER_OFFSET;
			else
				offset = DMA1_REGISTER_OFFSET;
			orig = data = RREG32(mmDMA_POWER_CNTL + offset);
			data &= ~DMA_POWER_CNTL__MEM_POWER_OVERRIDE_MASK;
			if (data != orig)
				WREG32(mmDMA_POWER_CNTL + offset, data);
			WREG32(mmDMA_CLK_CTRL + offset, 0x00000100);
		}
	} else {
		for (i = 0; i < 2; i++) {
			if (i == 0)
				offset = DMA0_REGISTER_OFFSET;
			else
				offset = DMA1_REGISTER_OFFSET;
			orig = data = RREG32(mmDMA_POWER_CNTL + offset);
			data |= DMA_POWER_CNTL__MEM_POWER_OVERRIDE_MASK;
			if (data != orig)
				WREG32(mmDMA_POWER_CNTL + offset, data);

			orig = data = RREG32(mmDMA_CLK_CTRL + offset);
			data = 0xff000000;
			if (data != orig)
				WREG32(mmDMA_CLK_CTRL + offset, data);
		}
	}
}

static int si_dma_early_init(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	adev->sdma.num_instances = DMA_MAX_INSTANCE;
    /* DEBUG */ printk(KERN_ALERT "SI DMA early init\n");

	si_dma_set_ring_funcs(adev);
	si_dma_set_irq_funcs(adev);
	si_dma_set_buffer_funcs(adev);
	si_dma_set_vm_pte_funcs(adev);

	return 0;
}

static int si_dma_sw_init(void *handle)
{
	struct amdgpu_ring *ring;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;
	int r, i;

    /* DEBUG */ printk(KERN_ALERT "SI DMA sw_init\n");
	/* DMA trap event */
	r = amdgpu_irq_add_id(adev, 224, &adev->sdma.trap_irq);
	if (r)
		return r;
    r = amdgpu_irq_add_id(adev, 244, &adev->sdma.trap_irq);
	if (r)
		return r;
	for (i = 0; i < adev->sdma.num_instances; i++) {
		ring = &adev->sdma.instance[i].ring;
		ring->ring_obj = NULL;
		sprintf(ring->name, "dma%d", i);
		r = amdgpu_ring_init(adev, ring, 256 * 1024,
				     SI_DMA_PACKET(DMA_PACKET_NOP, 0, 0, 0, 0), 0xf,
				     &adev->sdma.trap_irq,
				     (i == 0) ?
				     AMDGPU_SDMA_IRQ_TRAP0 : AMDGPU_SDMA_IRQ_TRAP1,
				     AMDGPU_RING_TYPE_SDMA);
		if (r)
			return r;
	}

	return r;
}

static int si_dma_sw_fini(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;
	int i;

    /* DEBUG */ printk(KERN_ALERT "SI DMA sw_fini\n");
	for (i = 0; i < adev->sdma.num_instances; i++)
		amdgpu_ring_fini(&adev->sdma.instance[i].ring);

	return 0;
}

static int si_dma_hw_init(void *handle)
{
	int r;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

    /* DEBUG */ printk(KERN_ALERT "SI DMA hw_init\n");
	r = si_dma_start(adev);
	if (r)
		return r;

    /* DEBUG */ printk(KERN_ALERT "SI DMA hw_init end\n");
	return r;
}

static int si_dma_hw_fini(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

    /* DEBUG */ printk(KERN_ALERT "SI DMA hw_fini\n");
	si_dma_enable(adev, false);

    /* DEBUG */ printk(KERN_ALERT "SI DMA hw_fini end\n");
	return 0;
}

static int si_dma_suspend(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	return si_dma_hw_fini(adev);
}

static int si_dma_resume(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	return si_dma_hw_init(adev);
}

static bool si_dma_is_idle(void *handle)
{
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;
	u32 tmp = RREG32(mmSRBM_STATUS2);

	if (tmp & (SRBM_STATUS2__DMA_BUSY_MASK |
				SRBM_STATUS2__DMA1_BUSY_MASK))
	    return false;

	return true;
}

static int si_dma_wait_for_idle(void *handle)
{
	unsigned i;
	u32 tmp;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

    /* DEBUG */ printk(KERN_ALERT "SI DMA wait for idle\n");
	for (i = 0; i < adev->usec_timeout; i++) {
		tmp = RREG32(mmSRBM_STATUS2) & (SRBM_STATUS2__DMA_BUSY_MASK |
				SRBM_STATUS2__DMA1_BUSY_MASK);

		if (!tmp)
			return 0;
		udelay(1);
	}
	return -ETIMEDOUT;
}

static void si_dma_print_status(void *handle)
{
	int i, j;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	dev_info(adev->dev, "SI DMA registers\n");
	dev_info(adev->dev, "  SRBM_STATUS2=0x%08X\n",
		 RREG32(mmSRBM_STATUS2));
	for (i = 0; i < adev->sdma.num_instances; i++) {

	}
}

static int si_dma_soft_reset(void *handle)
{
	u32 srbm_soft_reset = 0;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;
	u32 tmp = RREG32(mmSRBM_STATUS2);

    /* DEBUG */ printk(KERN_ALERT "SI DMA soft reset\n");
	if (tmp & SRBM_STATUS2__DMA_BUSY_MASK) {
		/* dma0 */
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_DMA_MASK;
	}
	if (tmp & SRBM_STATUS2__DMA1_BUSY_MASK) {
		/* dma1 */
		srbm_soft_reset |= SRBM_SOFT_RESET__SOFT_RESET_DMA1_MASK;
	}

	if (srbm_soft_reset) {
		si_dma_print_status((void *)adev);

		tmp = RREG32(mmSRBM_SOFT_RESET);
		tmp |= srbm_soft_reset;
		dev_info(adev->dev, "SRBM_SOFT_RESET=0x%08X\n", tmp);
		WREG32(mmSRBM_SOFT_RESET, tmp);
		tmp = RREG32(mmSRBM_SOFT_RESET);

		udelay(50);

		tmp &= ~srbm_soft_reset;
		WREG32(mmSRBM_SOFT_RESET, tmp);
		tmp = RREG32(mmSRBM_SOFT_RESET);

		/* Wait a little for things to settle down */
		udelay(50);

		si_dma_print_status((void *)adev);
	}

	return 0;
}

static int si_dma_set_trap_irq_state(struct amdgpu_device *adev,
				       struct amdgpu_irq_src *src,
				       unsigned type,
				       enum amdgpu_interrupt_state state)
{
	u32 dma_cntl;
    /* DEBUG */ printk(KERN_ALERT "Set trap IRQ state %d %d\n", type, (unsigned)state);

	switch (type) {
	case AMDGPU_SDMA_IRQ_TRAP0:
		switch (state) {
		case AMDGPU_IRQ_STATE_DISABLE:
			dma_cntl = RREG32(mmDMA_CNTL + DMA0_REGISTER_OFFSET);
			dma_cntl &= ~DMA_CNTL__TRAP_ENABLE_MASK;
			WREG32(mmDMA_CNTL + DMA0_REGISTER_OFFSET, dma_cntl);
			break;
		case AMDGPU_IRQ_STATE_ENABLE:
			/* DEBUG */ printk(KERN_ALERT "Enable trap0\n");
			dma_cntl = RREG32(mmDMA_CNTL + DMA0_REGISTER_OFFSET);
			dma_cntl |= DMA_CNTL__TRAP_ENABLE_MASK;
			WREG32(mmDMA_CNTL + DMA0_REGISTER_OFFSET, dma_cntl);
			break;
		default:
			break;
		}
		break;
	case AMDGPU_SDMA_IRQ_TRAP1:
		switch (state) {
		case AMDGPU_IRQ_STATE_DISABLE:
			dma_cntl = RREG32(mmDMA_CNTL + DMA1_REGISTER_OFFSET);
			dma_cntl &= ~DMA_CNTL__TRAP_ENABLE_MASK;
			WREG32(mmDMA_CNTL + DMA1_REGISTER_OFFSET, dma_cntl);
			break;
		case AMDGPU_IRQ_STATE_ENABLE:
			/* DEBUG */ printk(KERN_ALERT "Enable trap1\n");
			dma_cntl = RREG32(mmDMA_CNTL + DMA1_REGISTER_OFFSET);
			dma_cntl |= DMA_CNTL__TRAP_ENABLE_MASK;
			WREG32(mmDMA_CNTL + DMA1_REGISTER_OFFSET, dma_cntl);
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return 0;
}

static int si_dma_process_trap_irq(struct amdgpu_device *adev,
				     struct amdgpu_irq_src *source,
				     struct amdgpu_iv_entry *entry)
{
	u8 instance_id, queue_id;

	instance_id = (entry->ring_id & 0x3) >> 0;
	queue_id = (entry->ring_id & 0xc) >> 2;
    /* DEBUG */ printk(KERN_ALERT "SI DMA instance id: %d queue id: %d\n", instance_id, queue_id);
	DRM_DEBUG("IH: DMA trap\n");
	switch (instance_id) {
	case 0:
		switch (queue_id) {
		case 0:
			amdgpu_fence_process(&adev->sdma.instance[0].ring);
			break;
		case 1:
			/* XXX compute */
			break;
		case 2:
			/* XXX compute */
			break;
		}
		break;
	case 1:
		switch (queue_id) {
		case 0:
			amdgpu_fence_process(&adev->sdma.instance[1].ring);
			break;
		case 1:
			/* XXX compute */
			break;
		case 2:
			/* XXX compute */
			break;
		}
		break;
	}

	return 0;
}

static int si_dma_set_clockgating_state(void *handle,
					  enum amd_clockgating_state state)
{
	bool gate = false;
	struct amdgpu_device *adev = (struct amdgpu_device *)handle;

	if (state == AMD_CG_STATE_GATE)
		gate = true;

	si_enable_dma_mgcg(adev, gate);

	return 0;
}

static int si_dma_set_powergating_state(void *handle,
					  enum amd_powergating_state state)
{
	return 0;
}

const struct amd_ip_funcs si_dma_ip_funcs = {
	.early_init = si_dma_early_init,
	.late_init = NULL,
	.sw_init = si_dma_sw_init,
	.sw_fini = si_dma_sw_fini,
	.hw_init = si_dma_hw_init,
	.hw_fini = si_dma_hw_fini,
	.suspend = si_dma_suspend,
	.resume = si_dma_resume,
	.is_idle = si_dma_is_idle,
	.wait_for_idle = si_dma_wait_for_idle,
	.soft_reset = si_dma_soft_reset,
	.print_status = si_dma_print_status,
	.set_clockgating_state = si_dma_set_clockgating_state,
	.set_powergating_state = si_dma_set_powergating_state,
};

static const struct amdgpu_ring_funcs si_dma_ring_funcs = {
	.get_rptr = si_dma_ring_get_rptr,
	.get_wptr = si_dma_ring_get_wptr,
	.set_wptr = si_dma_ring_set_wptr,
	.parse_cs = NULL,
	.emit_ib = si_dma_ring_emit_ib,
	.emit_fence = si_dma_ring_emit_fence,
	.emit_semaphore = si_dma_ring_emit_semaphore,
	.emit_vm_flush = si_dma_ring_emit_vm_flush,
	.emit_hdp_flush = si_dma_ring_emit_hdp_flush,
	.test_ring = si_dma_ring_test_ring,
	.test_ib = si_dma_ring_test_ib,
	.insert_nop = si_dma_ring_insert_nop,
};

static void si_dma_set_ring_funcs(struct amdgpu_device *adev)
{
	int i;

	for (i = 0; i < adev->sdma.num_instances; i++)
		adev->sdma.instance[i].ring.funcs = &si_dma_ring_funcs;
}

static const struct amdgpu_irq_src_funcs si_dma_trap_irq_funcs = {
	.set = si_dma_set_trap_irq_state,
	.process = si_dma_process_trap_irq,
};

static void si_dma_set_irq_funcs(struct amdgpu_device *adev)
{
	adev->sdma.trap_irq.num_types = AMDGPU_SDMA_IRQ_LAST;
	adev->sdma.trap_irq.funcs = &si_dma_trap_irq_funcs;
}

/**
 * si_sdma_emit_copy_buffer - copy buffer using the sDMA engine
 *
 * @ring: amdgpu_ring structure holding ring information
 * @src_offset: src GPU address
 * @dst_offset: dst GPU address
 * @byte_count: number of bytes to xfer
 *
 * Copy GPU buffers using the DMA engine (SI).
 * Used by the amdgpu ttm implementation to move pages if
 * registered as the asic copy callback.
 */
static void si_dma_emit_copy_buffer(struct amdgpu_ib *ib,
				      uint64_t src_offset,
				      uint64_t dst_offset,
				      uint32_t byte_count)
{
    ib->ptr[ib->length_dw++] = SI_DMA_PACKET(DMA_PACKET_COPY, 1, 0, 0, byte_count);
    ib->ptr[ib->length_dw++] = lower_32_bits(dst_offset);
    ib->ptr[ib->length_dw++] = lower_32_bits(src_offset);
    ib->ptr[ib->length_dw++] = upper_32_bits(dst_offset) & 0xff;
    ib->ptr[ib->length_dw++] = upper_32_bits(src_offset) & 0xff;
}

/**
 * cik_sdma_emit_fill_buffer - fill buffer using the sDMA engine
 *
 * @ring: amdgpu_ring structure holding ring information
 * @src_data: value to write to buffer
 * @dst_offset: dst GPU address
 * @byte_count: number of bytes to xfer
 *
 * Fill GPU buffers using the DMA engine (SI).
 */
static void si_dma_emit_fill_buffer(struct amdgpu_ib *ib,
				      uint32_t src_data,
				      uint64_t dst_offset,
				      uint32_t byte_count)
{
    ib->ptr[ib->length_dw++] = SI_DMA_PACKET(DMA_PACKET_CONSTANT_FILL, 0, 0, 0, (byte_count >> 2));
    ib->ptr[ib->length_dw++] = lower_32_bits(dst_offset);
    ib->ptr[ib->length_dw++] = src_data;
    ib->ptr[ib->length_dw++] = upper_32_bits(dst_offset);
}

static const struct amdgpu_buffer_funcs si_dma_buffer_funcs = {
	.copy_max_bytes = 0xfffff,
	.copy_num_dw = 5,
	.emit_copy_buffer = si_dma_emit_copy_buffer,

	.fill_max_bytes = 0xfffff << 2,
	.fill_num_dw = 4,
	.emit_fill_buffer = si_dma_emit_fill_buffer,
};

static void si_dma_set_buffer_funcs(struct amdgpu_device *adev)
{
	if (adev->mman.buffer_funcs == NULL) {
		adev->mman.buffer_funcs = &si_dma_buffer_funcs;
		adev->mman.buffer_funcs_ring = &adev->sdma.instance[0].ring;
	}
}

static const struct amdgpu_vm_pte_funcs si_dma_vm_pte_funcs = {
	.copy_pte = si_dma_vm_copy_pte,
	.write_pte = si_dma_vm_write_pte,
	.set_pte_pde = si_dma_vm_set_pte_pde,
	.pad_ib = si_dma_vm_pad_ib,
};


static void si_dma_set_vm_pte_funcs(struct amdgpu_device *adev)
{
	if (adev->vm_manager.vm_pte_funcs == NULL) {
		adev->vm_manager.vm_pte_funcs = &si_dma_vm_pte_funcs;
		adev->vm_manager.vm_pte_funcs_ring = &adev->sdma.instance[0].ring;
		adev->vm_manager.vm_pte_funcs_ring->is_pte_ring = true;
	}
}
