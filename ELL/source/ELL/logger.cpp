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

#include <UVE/utils/logger.hpp>

#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <memory>

logger::logger( ) : log( "Luciole" )
{
   auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>( );
   console_sink->set_level( spdlog::level::debug );
   console_sink->set_pattern( "[%H:%M:%S.%f] [%n] [%l] %v" );

   auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>( "logs.txt", true );
   file_sink->set_level( spdlog::level::trace );
   file_sink->set_pattern( "[%H:%M:%S.%f] [%n] [%l] %v" );

   log = spdlog::logger( "Luciole", {console_sink, file_sink} );
   log.set_level( spdlog::level::debug );
}

logger::logger( std::string_view name ) : log( std::string( name ) )
{
   auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>( );
   console_sink->set_level( spdlog::level::debug );
   console_sink->set_pattern( "[%H:%M:%S.%f] [%n] [%l] %v" );

   auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>( "logs.txt", true );
   file_sink->set_level( spdlog::level::trace );
   file_sink->set_pattern( "[%H:%M:%S.%f] [%n] [%l] %v" );

   log = spdlog::logger( std::string( name ), {console_sink, file_sink} );
}

spdlog::logger& logger::get_logger( )
{
   return log;
}
