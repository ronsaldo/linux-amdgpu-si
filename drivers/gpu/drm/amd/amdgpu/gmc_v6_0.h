#ifndef __GMC_V6_0_H__
#define __GMC_V6_0_H__

extern const struct amd_ip_funcs gmc_v6_0_ip_funcs;

/* XXX these shouldn't be exported */
void gmc_v6_0_mc_stop(struct amdgpu_device *adev,
		      struct amdgpu_mode_mc_save *save);
void gmc_v6_0_mc_resume(struct amdgpu_device *adev,
			struct amdgpu_mode_mc_save *save);
int gmc_v6_0_mc_wait_for_idle(struct amdgpu_device *adev);

#endif
