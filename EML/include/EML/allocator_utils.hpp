#pragma once

#include <cstddef>
#include <cstdint>

namespace EML
{ 
   constexpr std::size_t get_backward_padding( std::uintptr_t address, std::size_t aligment ) noexcept
   {
      auto const padding = address & ( aligment - 1 );

      return padding == aligment ? 0 : padding;
   }

   constexpr std::size_t get_forward_padding( std::uintptr_t address, std::size_t alignment ) noexcept
   {
      auto const padding = alignment - ( address & ( alignment - 1 ) );

      return padding == alignment ? 0 : padding;
   }

   constexpr std::size_t get_forward_padding( std::uintptr_t address, std::size_t alignment, std::size_t header_size ) noexcept
   {
      auto padding = get_forward_padding( address, alignment );

      if ( padding < header_size )
      {
         auto const needed_space = header_size - padding;

         padding += alignment * ( needed_space / alignment );
         if ( needed_space % alignment > 0 )
         {
            padding += alignment;
         }
      }

      return padding;
   }
}
