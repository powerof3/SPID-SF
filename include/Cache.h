#pragma once

namespace Cache::FormType
{
	inline constexpr std::array whitelist{
	    RE::FormType::kFACT,
		RE::FormType::kCLAS,
		RE::FormType::kCSTY,
		RE::FormType::kRACE,
		RE::FormType::kOTFT,
		RE::FormType::kNPC_,
		RE::FormType::kVTYP,
		RE::FormType::kFLST
	};

	bool GetWhitelisted(RE::FormType a_type);
}
