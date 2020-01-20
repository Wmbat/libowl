
#include <EML/allocators/linear_allocator.hpp>
#include <EML/allocators/stack_allocator.hpp>
#include <UVE/core/context.hpp>
#include <UVE/utils/logger.hpp>
#include <UVE/ui/window.hpp>

int main( )
{
   logger main_logger( "main_logger" );

   auto test_allocator = EML::stack_allocator<1024>( );
   auto* p_test = test_allocator.allocate( sizeof( int ), alignof( int ) );

   auto create_info = UVE::window_create_info{ };
   create_info.position = glm::uvec2( 0, 0 );
   create_info.size = glm::uvec2( 1280, 720 );
   create_info.title = "Triangle Example";
   create_info.p_logger = &main_logger;

   UVE::window window( create_info );
   UVE::context context( &main_logger );
   
   auto p_render_target = context.create_render_target( window );

   context.find_best_physical_device( p_render_target ); 
   
   return 0;
}
