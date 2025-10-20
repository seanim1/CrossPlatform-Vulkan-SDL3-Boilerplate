#include "VulkanSpecializationConstant.h"


VulkanSpecializationConstant::VulkanSpecializationConstant(
	int Screen_Width,
	int Screen_Heigt,
	int model_id
)
{
	gpuConstantData.Screen_Width = Screen_Width;
	gpuConstantData.Screen_Heigt = Screen_Heigt;
	gpuConstantData.model_id = model_id;
	// Specialization Constant (Compile-time Constant used for compiler optimization) 
	// The constant_id can only be applied to a scalar *int*, a scalar *float* or a scalar * bool*
	// Map constant IDs to offsets in the buffer
	for (int i = 0; i < specialization_constant_count; i++) {
		specializationMapEntries[i].constantID = i; // 
		specializationMapEntries[i].offset = i * 4; // Offset in the specialization data buffer
		specializationMapEntries[i].size = sizeof(int); // Size of the data
	}
	// Define specialization info
	specializationInfo.mapEntryCount = specialization_constant_count;
	specializationInfo.pMapEntries = (VkSpecializationMapEntry*)specializationMapEntries; // Pointer to the map entry
	specializationInfo.dataSize = sizeof(gpuConstantData);	// Size of the specialization data
	specializationInfo.pData = &gpuConstantData;	// Pointer to the data
}
