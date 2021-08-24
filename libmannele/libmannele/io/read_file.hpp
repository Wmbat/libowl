#ifndef LIBMANNELE_IO_READ_FILE_HPP
#define LIBMANNELE_IO_READ_FILE_HPP

#include <libmannele/error/runtime_error.hpp>

#include <libreglisse/result.hpp>

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
      -> reglisse::result<std::string, runtime_error>;
} // namespace mannele

#endif // LIBMANNELE_IO_READ_FILE_HPP
