#pragma once

namespace Utils {

	template<typename T>
	static RenderPipeline* Create(VkDevice device) {
		static_assert(std::is_base_of<RenderPipeline, T>::value, "T Is Not a RenderPipeline!");

		auto _ptr = new T();

		auto _pipeline = reinterpret_cast<RenderPipeline*>(_ptr);
		_pipeline->LinkDevice(device);

		return _ptr;
	}


}
