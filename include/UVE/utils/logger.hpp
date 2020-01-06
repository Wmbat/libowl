/*
 *  Copyright (C) 2018-2019 Wmbat
 *
 *  wmbat@protonmail.com
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  You should have received a copy of the GNU General Public License
 *  GNU General Public License for more details.
 *  along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LUCIOLE_UTILS_LOGGER_HPP
#define LUCIOLE_UTILS_LOGGER_HPP

#include <spdlog/spdlog.h>

class logger
{
public:
   logger( );
   logger( std::string_view name );

   void info( std::string const& msg ) { log.info( msg ); }

   void debug( std::string const& msg ) { log.debug( msg ); }

   void warn( std::string const& msg ) { log.warn( msg ); }

   void error( std::string const& msg ) { log.error( msg ); }

   void flush( ) { log.flush( ); }

   template <typename... _args>
   void info( std::string_view msg, _args const&... args )
   {
      log.info( msg, args... );
   }

   template <typename... _args>
   void debug( std::string_view msg, _args const&... args )
   {
      log.debug( msg, args... );
   }

   template <typename... _args>
   void warn( std::string_view msg, _args const&... args )
   {
      log.warn( msg, args... );
   }

   template <typename... _args>
   void error( std::string_view msg, _args const&... args )
   {
      log.error( msg, args... );
   }

   spdlog::logger& get_logger( );

private:
   spdlog::logger log;
};

#endif // LUCIOLE_UTILS_LOGGER_HPP
