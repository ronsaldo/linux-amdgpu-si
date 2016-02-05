/*
 * Copyright 2011 Advanced Micro Devices, Inc.
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
 * Authors: Alex Deucher
 */
#ifndef SI_H
#define SI_H

#define TAHITI_RB_BITMAP_WIDTH_PER_SH  2

#define MC_SEQ_MISC0__MT__MASK	0xf0000000
#define MC_SEQ_MISC0__MT__GDDR1  0x10000000
#define MC_SEQ_MISC0__MT__DDR2   0x20000000
#define MC_SEQ_MISC0__MT__GDDR3  0x30000000
#define MC_SEQ_MISC0__MT__GDDR4  0x40000000
#define MC_SEQ_MISC0__MT__GDDR5  0x50000000
#define MC_SEQ_MISC0__MT__HBM    0x60000000
#define MC_SEQ_MISC0__MT__DDR3   0xB0000000

#define TAHITI_GB_ADDR_CONFIG_GOLDEN        0x12011003
#define VERDE_GB_ADDR_CONFIG_GOLDEN         0x12010002
#define HAINAN_GB_ADDR_CONFIG_GOLDEN        0x02010001

#define AMDGPU_NUM_OF_VMIDS	8

#define SI_MAX_SH_GPRS           256
#define SI_MAX_TEMP_GPRS         16
#define SI_MAX_SH_THREADS        256
#define SI_MAX_SH_STACK_ENTRIES  4096
#define SI_MAX_FRC_EOV_CNT       16384
#define SI_MAX_BACKENDS          8
#define SI_MAX_BACKENDS_MASK     0xFF
#define SI_MAX_BACKENDS_PER_SE_MASK     0x0F
#define SI_MAX_SIMDS             12
#define SI_MAX_SIMDS_MASK        0x0FFF
#define SI_MAX_SIMDS_PER_SE_MASK        0x00FF
#define SI_MAX_PIPES             8
#define SI_MAX_PIPES_MASK        0xFF
#define SI_MAX_PIPES_PER_SIMD_MASK      0x3F
#define SI_MAX_LDS_NUM           0xFFFF
#define SI_MAX_TCC               16
#define SI_MAX_TCC_MASK          0xFFFF

/*
 * PM4
 */
#define PACKET0(reg, n)	((RADEON_PACKET_TYPE0 << 30) |			\
			 (((reg) >> 2) & 0xFFFF) |			\
			 ((n) & 0x3FFF) << 16)
#define CP_PACKET2			0x80000000
#define		PACKET2_PAD_SHIFT		0
#define		PACKET2_PAD_MASK		(0x3fffffff << 0)

#define PACKET2(v)	(CP_PACKET2 | REG_SET(PACKET2_PAD, (v)))

#define PACKET3(op, n)	((RADEON_PACKET_TYPE3 << 30) |			\
			 (((op) & 0xFF) << 8) |				\
			 ((n) & 0x3FFF) << 16)

#define PACKET3_COMPUTE(op, n) (PACKET3(op, n) | 1 << 1)

/* Packet 3 types */
#define	PACKET3_NOP					0x10
#define	PACKET3_SET_BASE				0x11
#define		PACKET3_BASE_INDEX(x)                  ((x) << 0)
#define			GDS_PARTITION_BASE		2
#define			CE_PARTITION_BASE		3
#define	PACKET3_CLEAR_STATE				0x12
#define	PACKET3_INDEX_BUFFER_SIZE			0x13
#define	PACKET3_DISPATCH_DIRECT				0x15
#define	PACKET3_DISPATCH_INDIRECT			0x16
#define	PACKET3_ALLOC_GDS				0x1B
#define	PACKET3_WRITE_GDS_RAM				0x1C
#define	PACKET3_ATOMIC_GDS				0x1D
#define	PACKET3_ATOMIC					0x1E
#define	PACKET3_OCCLUSION_QUERY				0x1F
#define	PACKET3_SET_PREDICATION				0x20
#define	PACKET3_REG_RMW					0x21
#define	PACKET3_COND_EXEC				0x22
#define	PACKET3_PRED_EXEC				0x23
#define	PACKET3_DRAW_INDIRECT				0x24
#define	PACKET3_DRAW_INDEX_INDIRECT			0x25
#define	PACKET3_INDEX_BASE				0x26
#define	PACKET3_DRAW_INDEX_2				0x27
#define	PACKET3_CONTEXT_CONTROL				0x28
#define	PACKET3_INDEX_TYPE				0x2A
#define	PACKET3_DRAW_INDIRECT_MULTI			0x2C
#define	PACKET3_DRAW_INDEX_AUTO				0x2D
#define	PACKET3_DRAW_INDEX_IMMD				0x2E
#define	PACKET3_NUM_INSTANCES				0x2F
#define	PACKET3_DRAW_INDEX_MULTI_AUTO			0x30
#define	PACKET3_INDIRECT_BUFFER_CONST			0x31
#define	PACKET3_INDIRECT_BUFFER				0x32
#define	PACKET3_STRMOUT_BUFFER_UPDATE			0x34
#define	PACKET3_DRAW_INDEX_OFFSET_2			0x35
#define	PACKET3_DRAW_INDEX_MULTI_ELEMENT		0x36
#define	PACKET3_WRITE_DATA				0x37
#define		WRITE_DATA_DST_SEL(x)                   ((x) << 8)
                /* 0 - register
		 * 1 - memory (sync - via GRBM)
		 * 2 - tc/l2
		 * 3 - gds
		 * 4 - reserved
		 * 5 - memory (async - direct)
		 */
#define		WR_ONE_ADDR                             (1 << 16)
#define		WR_CONFIRM                              (1 << 20)
#define		WRITE_DATA_ENGINE_SEL(x)                ((x) << 30)
                /* 0 - me
		 * 1 - pfp
		 * 2 - ce
		 */
#define	PACKET3_DRAW_INDEX_INDIRECT_MULTI		0x38
#define	PACKET3_MEM_SEMAPHORE				0x39
#define	PACKET3_MPEG_INDEX				0x3A
#define	PACKET3_COPY_DW					0x3B
#define	PACKET3_WAIT_REG_MEM				0x3C
#define		WAIT_REG_MEM_FUNCTION(x)                ((x) << 0)
                /* 0 - always
		 * 1 - <
		 * 2 - <=
		 * 3 - ==
		 * 4 - !=
		 * 5 - >=
		 * 6 - >
		 */
#define		WAIT_REG_MEM_MEM_SPACE(x)               ((x) << 4)
                /* 0 - reg
		 * 1 - mem
		 */
#define		WAIT_REG_MEM_ENGINE(x)                  ((x) << 8)
                /* 0 - me
		 * 1 - pfp
		 */
#define	PACKET3_MEM_WRITE				0x3D
#define	PACKET3_COPY_DATA				0x40
#define	PACKET3_CP_DMA					0x41
/* 1. header
 * 2. SRC_ADDR_LO or DATA [31:0]
 * 3. CP_SYNC [31] | SRC_SEL [30:29] | ENGINE [27] | DST_SEL [21:20] |
 *    SRC_ADDR_HI [7:0]
 * 4. DST_ADDR_LO [31:0]
 * 5. DST_ADDR_HI [7:0]
 * 6. COMMAND [30:21] | BYTE_COUNT [20:0]
 */
#              define PACKET3_CP_DMA_DST_SEL(x)    ((x) << 20)
                /* 0 - DST_ADDR
		 * 1 - GDS
		 */
#              define PACKET3_CP_DMA_ENGINE(x)     ((x) << 27)
                /* 0 - ME
		 * 1 - PFP
		 */
#              define PACKET3_CP_DMA_SRC_SEL(x)    ((x) << 29)
                /* 0 - SRC_ADDR
		 * 1 - GDS
		 * 2 - DATA
		 */
#              define PACKET3_CP_DMA_CP_SYNC       (1 << 31)
/* COMMAND */
#              define PACKET3_CP_DMA_DIS_WC        (1 << 21)
#              define PACKET3_CP_DMA_CMD_SRC_SWAP(x) ((x) << 22)
                /* 0 - none
		 * 1 - 8 in 16
		 * 2 - 8 in 32
		 * 3 - 8 in 64
		 */
#              define PACKET3_CP_DMA_CMD_DST_SWAP(x) ((x) << 24)
                /* 0 - none
		 * 1 - 8 in 16
		 * 2 - 8 in 32
		 * 3 - 8 in 64
		 */
#              define PACKET3_CP_DMA_CMD_SAS       (1 << 26)
                /* 0 - memory
		 * 1 - register
		 */
#              define PACKET3_CP_DMA_CMD_DAS       (1 << 27)
                /* 0 - memory
		 * 1 - register
		 */
#              define PACKET3_CP_DMA_CMD_SAIC      (1 << 28)
#              define PACKET3_CP_DMA_CMD_DAIC      (1 << 29)
#              define PACKET3_CP_DMA_CMD_RAW_WAIT  (1 << 30)
#define	PACKET3_PFP_SYNC_ME				0x42
#define	PACKET3_SURFACE_SYNC				0x43
#              define PACKET3_DEST_BASE_0_ENA      (1 << 0)
#              define PACKET3_DEST_BASE_1_ENA      (1 << 1)
#              define PACKET3_CB0_DEST_BASE_ENA    (1 << 6)
#              define PACKET3_CB1_DEST_BASE_ENA    (1 << 7)
#              define PACKET3_CB2_DEST_BASE_ENA    (1 << 8)
#              define PACKET3_CB3_DEST_BASE_ENA    (1 << 9)
#              define PACKET3_CB4_DEST_BASE_ENA    (1 << 10)
#              define PACKET3_CB5_DEST_BASE_ENA    (1 << 11)
#              define PACKET3_CB6_DEST_BASE_ENA    (1 << 12)
#              define PACKET3_CB7_DEST_BASE_ENA    (1 << 13)
#              define PACKET3_DB_DEST_BASE_ENA     (1 << 14)
#              define PACKET3_DEST_BASE_2_ENA      (1 << 19)
#              define PACKET3_DEST_BASE_3_ENA      (1 << 21)
#              define PACKET3_TCL1_ACTION_ENA      (1 << 22)
#              define PACKET3_TC_ACTION_ENA        (1 << 23)
#              define PACKET3_CB_ACTION_ENA        (1 << 25)
#              define PACKET3_DB_ACTION_ENA        (1 << 26)
#              define PACKET3_SH_KCACHE_ACTION_ENA (1 << 27)
#              define PACKET3_SH_ICACHE_ACTION_ENA (1 << 29)
#define	PACKET3_ME_INITIALIZE				0x44
#define		PACKET3_ME_INITIALIZE_DEVICE_ID(x) ((x) << 16)
#define	PACKET3_COND_WRITE				0x45
#define	PACKET3_EVENT_WRITE				0x46
#define		EVENT_TYPE(x)                           ((x) << 0)
#define		EVENT_INDEX(x)                          ((x) << 8)
                /* 0 - any non-TS event
		 * 1 - ZPASS_DONE
		 * 2 - SAMPLE_PIPELINESTAT
		 * 3 - SAMPLE_STREAMOUTSTAT*
		 * 4 - *S_PARTIAL_FLUSH
		 * 5 - EOP events
		 * 6 - EOS events
		 * 7 - CACHE_FLUSH, CACHE_FLUSH_AND_INV_EVENT
		 */
#define		INV_L2                                  (1 << 20)
                /* INV TC L2 cache when EVENT_INDEX = 7 */
#define	PACKET3_EVENT_WRITE_EOP				0x47
#define		DATA_SEL(x)                             ((x) << 29)
                /* 0 - discard
		 * 1 - send low 32bit data
		 * 2 - send 64bit data
		 * 3 - send 64bit counter value
		 */
#define		INT_SEL(x)                              ((x) << 24)
                /* 0 - none
		 * 1 - interrupt only (DATA_SEL = 0)
		 * 2 - interrupt when data write is confirmed
		 */
#define	PACKET3_EVENT_WRITE_EOS				0x48
#define	PACKET3_PREAMBLE_CNTL				0x4A
#              define PACKET3_PREAMBLE_BEGIN_CLEAR_STATE     (2 << 28)
#              define PACKET3_PREAMBLE_END_CLEAR_STATE       (3 << 28)
#define	PACKET3_ONE_REG_WRITE				0x57
#define	PACKET3_LOAD_CONFIG_REG				0x5F
#define	PACKET3_LOAD_CONTEXT_REG			0x60
#define	PACKET3_LOAD_SH_REG				0x61
#define	PACKET3_SET_CONFIG_REG				0x68
#define		PACKET3_SET_CONFIG_REG_START			0x00008000
#define		PACKET3_SET_CONFIG_REG_END			0x0000b000
#define	PACKET3_SET_CONTEXT_REG				0x69
#define		PACKET3_SET_CONTEXT_REG_START			0x00028000
#define		PACKET3_SET_CONTEXT_REG_END			0x00029000
#define	PACKET3_SET_CONTEXT_REG_INDIRECT		0x73
#define	PACKET3_SET_RESOURCE_INDIRECT			0x74
#define	PACKET3_SET_SH_REG				0x76
#define		PACKET3_SET_SH_REG_START			0x0000b000
#define		PACKET3_SET_SH_REG_END				0x0000c000
#define	PACKET3_SET_SH_REG_OFFSET			0x77
#define	PACKET3_ME_WRITE				0x7A
#define	PACKET3_SCRATCH_RAM_WRITE			0x7D
#define	PACKET3_SCRATCH_RAM_READ			0x7E
#define	PACKET3_CE_WRITE				0x7F
#define	PACKET3_LOAD_CONST_RAM				0x80
#define	PACKET3_WRITE_CONST_RAM				0x81
#define	PACKET3_WRITE_CONST_RAM_OFFSET			0x82
#define	PACKET3_DUMP_CONST_RAM				0x83
#define	PACKET3_INCREMENT_CE_COUNTER			0x84
#define	PACKET3_INCREMENT_DE_COUNTER			0x85
#define	PACKET3_WAIT_ON_CE_COUNTER			0x86
#define	PACKET3_WAIT_ON_DE_COUNTER			0x87
#define	PACKET3_WAIT_ON_DE_COUNTER_DIFF			0x88
#define	PACKET3_SET_CE_DE_COUNTERS			0x89
#define	PACKET3_WAIT_ON_AVAIL_BUFFER			0x8A
#define	PACKET3_SWITCH_BUFFER				0x8B

/* ASYNC DMA - first instance at 0xd000, second at 0xd800 */
#define DMA0_REGISTER_OFFSET                              0x0 /* not a register */
#define DMA1_REGISTER_OFFSET                              0x200 /* not a register */
#define DMA_MAX_INSTANCE 2

#define DMA_PACKET(cmd, b, t, s, n)	((((cmd) & 0xF) << 28) |	\
					 (((b) & 0x1) << 26) |		\
					 (((t) & 0x1) << 23) |		\
					 (((s) & 0x1) << 22) |		\
					 (((n) & 0xFFFFF) << 0))

#define DMA_IB_PACKET(cmd, vmid, n)	((((cmd) & 0xF) << 28) |	\
					 (((vmid) & 0xF) << 20) |	\
					 (((n) & 0xFFFFF) << 0))

#define DMA_PTE_PDE_PACKET(n)		((2 << 28) |			\
					 (1 << 26) |			\
					 (1 << 21) |			\
					 (((n) & 0xFFFFF) << 0))

/* async DMA Packet types */
#define	DMA_PACKET_WRITE				  0x2
#define	DMA_PACKET_COPY					  0x3
#define	DMA_PACKET_INDIRECT_BUFFER			  0x4
#define	DMA_PACKET_SEMAPHORE				  0x5
#define	DMA_PACKET_FENCE				  0x6
#define	DMA_PACKET_TRAP					  0x7
#define	DMA_PACKET_SRBM_WRITE				  0x9
#define	DMA_PACKET_CONSTANT_FILL			  0xd
#define	DMA_PACKET_POLL_REG_MEM				  0xe
#define	DMA_PACKET_NOP					  0xf

#define VCE_CMD_NO_OP					0x00000000
#define VCE_CMD_END					0x00000001
#define VCE_CMD_IB					0x00000002
#define VCE_CMD_FENCE					0x00000003
#define VCE_CMD_TRAP					0x00000004
#define VCE_CMD_IB_AUTO					0x00000005
#define VCE_CMD_SEMAPHORE				0x00000006

#endif
