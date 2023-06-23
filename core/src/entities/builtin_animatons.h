#pragma once

// animate linearly any numeric value
template<typename T>
T Animate_Linear(T source, T target, double progress) {
	if constexpr (std::is_integral_v<std::remove_cvref_t<T>> || std::is_floating_point_v<std::remove_cvref_t<T>>) {
		return static_cast<T>(static_cast<const double>(source) + static_cast<const double>(target - source) * progress);
	}

	return target;
}
