
#include <UVE/core/context.hpp>
#include <UVE/utils/logger.hpp>

int main( )
{
   logger main_logger( "main_logger" );

   UVE::context context( &main_logger );

   return 0;
}
