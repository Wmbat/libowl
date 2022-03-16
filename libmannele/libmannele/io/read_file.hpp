/**
 * @file libmannele/io/read_file.hpp
 * @author wmbat wmbat-dev@protonmail.com
 * @date Monday, 22nd of September 2021
 * @brief 
 * @copyright Copyright (C) 2021 wmbat.
 */

#ifndef LIBMANNELE_IO_READ_FILE_HPP_
#define LIBMANNELE_IO_READ_FILE_HPP_

#include <libmannele/error/runtime_error.hpp>

#include <tl/expected.hpp>

#include <filesystem>
#include <string>

namespace mannele
{
   enum class file_reading_error
   {
      e_failed_to_open_file
   };

   auto make_error_condition(file_reading_error e) -> std::error_condition;

   auto unbuffered_file_read(const std::filesystem::path& path)
      -> tl::expected<std::string, runtime_error>;
} // namespace mannele

#endif // LIBMANNELE_IO_READ_FILE_HPP_
