/**
 * @file libmannele/io/read_file.cpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 22nd of September 2021
 * @brief
 * @copyright Copyright (C) 2021 wmbat.
 */

#include <libmannele/io/read_file.hpp>

#include <libmannele/core.hpp>
#include <libmannele/error/runtime_error.hpp>

#include <magic_enum.hpp>

#include <fstream>

struct file_reading_error_category : std::error_category
{
   [[nodiscard]] auto name() const noexcept -> const char* override { return "file_reading"; }
   [[nodiscard]] auto message(int err) const -> std::string override
   {
      return std::string(magic_enum::enum_name(static_cast<mannele::file_reading_error>(err)));
   }
};

inline static const file_reading_error_category file_reading_error_cat{};

namespace mannele
{
   auto make_error_condition(file_reading_error e) -> std::error_condition
   {
      return std::error_condition({static_cast<int>(e), file_reading_error_cat});
   }

   auto unbuffered_file_read(const std::filesystem::path& path)
      -> tl::expected<std::string, runtime_error>
   {
      auto input = std::ifstream(path);

      if (!input.is_open())
      {
         return tl::unexpected(
            runtime_error(make_error_condition(file_reading_error::e_failed_to_open_file)));
      }

      input.seekg(0, std::ios::end);

      auto data = std::string();
      data.reserve(static_cast<u64>(input.tellg()));

      input.seekg(0, std::ios::beg);

      data.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());

      return data;
   }
} // namespace mannele
