/*!
 * @author wmbat@protonmail.com
 *
 * Copyright (C) 2019 Wmbat
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * You should have received a copy of the GNU General Public License
 * GNU General Public License for more details.
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LUCIOLE_UTILITIES_FILE_IO_H
#define LUCIOLE_UTILITIES_FILE_IO_H

#include <fstream>
#include <string_view>
#include <vector>

inline const std::string read_from_file( const std::string_view filepath )
{
   std::ifstream file( std::string{filepath} );
   std::string str;

   if ( !file.is_open( ) )
   {
      throw std::runtime_error{"Error loading file at location: " + std::string{filepath} + "."};
   }
   else if ( !file.good( ) )
   {
      throw std::runtime_error{"Error reading file: " + std::string{filepath} + "."};
   }

   char c;
   while ( file.get( c ) )
   {
      str.push_back( c );
   }

   return str;
}

inline const std::string read_from_binary_file( const std::string_view filepath )
{
   std::ifstream file( std::string{filepath}, std::ios::binary );
   std::string str;

   if ( !file.is_open( ) )
   {
      throw std::runtime_error{"Error loading file at location: " + std::string{filepath} + "."};
   }
   else if ( !file.good( ) )
   {
      throw std::runtime_error{"Error reading file: " + std::string{filepath} + "."};
   }

   char c;
   while ( file.get( c ) )
   {
      str.push_back( c );
   }

   return str;
}

inline void write_to_file( const std::string& filepath, const std::string& data )
{
   std::ofstream file( filepath, std::ios::binary );

   if ( !file.good( ) )
   {
      throw std::runtime_error{"Error finding file: " + filepath + "."};
   }

   file << data;
}

#endif // BAZAAR_UTILITIES_FILE_IO_HPP
