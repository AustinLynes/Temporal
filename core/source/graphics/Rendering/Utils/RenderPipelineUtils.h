#pragma once

namespace Utils {

	template<typename T>
	static RenderPipeline* Create() {
		static_assert(std::is_base_of<RenderPipeline, T>::value, "T Is Not a RenderPipeline!");

		auto _ptr = new T();

		return _ptr;
	}


}
