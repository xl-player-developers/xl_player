cmake_minimum_required(VERSION 3.4.1)

set(lib_src_DIR ${CMAKE_SOURCE_DIR}/ff_libs/${ANDROID_ABI})
include_directories(${CMAKE_SOURCE_DIR}/../jni_include)

MESSAGE(STATUS "XL_PLayer : ANDROID_ABI  ${ANDROID_ABI}")

add_library(avcodec SHARED IMPORTED)
set_target_properties(avcodec PROPERTIES IMPORTED_LOCATION ${lib_src_DIR}/libavcodec-57.so)

add_library(avformat SHARED IMPORTED)
set_target_properties(avformat PROPERTIES IMPORTED_LOCATION ${lib_src_DIR}/libavformat-57.so)

add_library(avutil SHARED IMPORTED)
set_target_properties(avutil PROPERTIES IMPORTED_LOCATION ${lib_src_DIR}/libavutil-55.so)

add_library(avfilter SHARED IMPORTED)
set_target_properties(avfilter PROPERTIES IMPORTED_LOCATION ${lib_src_DIR}/libavfilter-6.so)

add_library(swresample SHARED IMPORTED)
set_target_properties(swresample PROPERTIES IMPORTED_LOCATION ${lib_src_DIR}/libswresample-2.so)

add_library(
        xl_render
        SHARED

        src/main/cpp/xl_audio/xl_audio_player.h
        src/main/cpp/xl_audio/xl_audio_player.c
        src/main/cpp/xl_audio/xl_audio_filter.h
        src/main/cpp/xl_audio/xl_audio_filter.c

        src/main/cpp/xl_container/xl_packet_queue.h
        src/main/cpp/xl_container/xl_packet_queue.c
        src/main/cpp/xl_container/xl_packet_pool.h
        src/main/cpp/xl_container/xl_packet_pool.c
        src/main/cpp/xl_container/xl_frame_queue.h
        src/main/cpp/xl_container/xl_frame_queue.c
        src/main/cpp/xl_container/xl_frame_pool.h
        src/main/cpp/xl_container/xl_frame_pool.c

        src/main/cpp/xl_decoders/xl_player_video_sw_decode_thread.h
        src/main/cpp/xl_decoders/xl_player_video_sw_decode_thread.c
        src/main/cpp/xl_decoders/xl_player_video_hw_decode_thread.h
        src/main/cpp/xl_decoders/xl_player_video_hw_decode_thread.c
        src/main/cpp/xl_decoders/xl_player_audio_decode_thread.h
        src/main/cpp/xl_decoders/xl_player_audio_decode_thread.c
        src/main/cpp/xl_decoders/xl_mediacodec.h
        src/main/cpp/xl_decoders/xl_mediacodec.c

        src/main/cpp/xl_video/xl_tracker.h
        src/main/cpp/xl_video/xl_tracker.c
        src/main/cpp/xl_video/xl_mat4.h
        src/main/cpp/xl_video/xl_mat4.c
        src/main/cpp/xl_video/xl_model.h
        src/main/cpp/xl_video/xl_model.c
        src/main/cpp/xl_video/xl_mesh_factory.h
        src/main/cpp/xl_video/xl_mesh_factory.c
        src/main/cpp/xl_video/xl_model_ball.h
        src/main/cpp/xl_video/xl_model_ball.c
        src/main/cpp/xl_video/xl_model_rect.h
        src/main/cpp/xl_video/xl_model_rect.c
        src/main/cpp/xl_video/xl_model_vr.h
        src/main/cpp/xl_video/xl_model_vr.c
        src/main/cpp/xl_video/xl_texture.h
        src/main/cpp/xl_video/xl_texture.c
        src/main/cpp/xl_video/xl_glsl_program.h
        src/main/cpp/xl_video/xl_glsl_program.c
        src/main/cpp/xl_video/xl_video_render.h
        src/main/cpp/xl_video/xl_video_render.c
        src/main/cpp/xl_video/xl_player_gl_thread.h
        src/main/cpp/xl_video/xl_player_gl_thread.c

        src/main/cpp/xl_types/xl_macro.h
        src/main/cpp/xl_types/xl_player_types.h
        src/main/cpp/xl_types/xl_video_render_types.h

        src/main/cpp/xl_utils/xl_statistics.h
        src/main/cpp/xl_utils/xl_statistics.c
        src/main/cpp/xl_utils/xl_clock.h
        src/main/cpp/xl_utils/xl_clock.c
        src/main/cpp/xl_utils/xl_jni_reflect.h
        src/main/cpp/xl_utils/xl_jni_reflect.c

        src/main/cpp/xl_player.c
        src/main/cpp/xl_playerCore.h
        src/main/cpp/xl_playerCore.c
        src/main/cpp/xl_player_read_thread.h
        src/main/cpp/xl_player_read_thread.c


)
add_library(
        ekf
        SHARED
        src/main/cpp/xl_head_tracker/Vector3d.h
        src/main/cpp/xl_head_tracker/Vector3d.cpp
        src/main/cpp/xl_head_tracker/SO3Util.h
        src/main/cpp/xl_head_tracker/SO3Util.cpp
        src/main/cpp/xl_head_tracker/Matrix3x3d.h
        src/main/cpp/xl_head_tracker/Matrix3x3d.cpp
        src/main/cpp/xl_head_tracker/OrientationEKF.h
        src/main/cpp/xl_head_tracker/OrientationEKF.cpp
        src/main/cpp/xl_head_tracker/HeadTracker.h
        src/main/cpp/xl_head_tracker/HeadTracker.cpp
)

find_library(log-lib log)
find_library(android-lib android)
find_library(bmp jnigraphics)
find_library(glesv2-lib GLESv2)
find_library(egl-lib EGL)
find_library(sles-lib OpenSLES)
if (${CMAKE_SYSTEM_VERSION} MATCHES "21")
    MESSAGE(STATUS "XL_PLayer : CMAKE_SYSTEM_VERSION  ${CMAKE_SYSTEM_VERSION} has mediandk")
    find_library(media-ndk mediandk)
    target_link_libraries(
            xl_render
            ekf
            avcodec
            avformat
            avutil
            avfilter
            swresample
            ${log-lib}
            ${bmp}
            ${android-lib}
            ${egl-lib}
            ${glesv2-lib}
            ${sles-lib}
            ${media-ndk}
    )
else (${CMAKE_SYSTEM_VERSION} LESS "21")
    MESSAGE(STATUS "XL_PLayer : CMAKE_SYSTEM_VERSION  ${CMAKE_SYSTEM_VERSION} not have mediandk")
    target_link_libraries(
            xl_render
            ekf
            avcodec
            avformat
            avutil
            avfilter
            swresample
            ${log-lib}
            ${bmp}
            ${android-lib}
            ${egl-lib}
            ${glesv2-lib}
            ${sles-lib}
    )
endif (${CMAKE_SYSTEM_VERSION} MATCHES "21")
