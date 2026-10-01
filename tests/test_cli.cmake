if(NOT DEFINED ARCHIVER OR NOT EXISTS "${ARCHIVER}")
    message(FATAL_ERROR "ARCHIVER must point to the built huffman_archiver executable")
endif()

file(REMOVE_RECURSE "${WORK_DIR}")
file(MAKE_DIRECTORY "${WORK_DIR}")

function(run_round_trip name content)
    set(input "${WORK_DIR}/${name}.input")
    set(archive "${WORK_DIR}/${name}.huf")
    set(output "${WORK_DIR}/${name}.output")
    file(WRITE "${input}" "${content}")

    execute_process(
        COMMAND "${ARCHIVER}" -c "${input}" "${archive}"
        RESULT_VARIABLE compress_result
        OUTPUT_QUIET
        ERROR_VARIABLE compress_error
    )
    if(NOT "${compress_result}" STREQUAL "0")
        message(FATAL_ERROR "Compression failed for ${name}: ${compress_error}")
    endif()

    execute_process(
        COMMAND "${ARCHIVER}" -d "${archive}" "${output}"
        RESULT_VARIABLE decompress_result
        OUTPUT_QUIET
        ERROR_VARIABLE decompress_error
    )
    if(NOT "${decompress_result}" STREQUAL "0")
        message(FATAL_ERROR "Decompression failed for ${name}: ${decompress_error}")
    endif()

    file(SHA256 "${input}" input_hash)
    file(SHA256 "${output}" output_hash)
    if(NOT input_hash STREQUAL output_hash)
        message(FATAL_ERROR "Round-trip mismatch for ${name}")
    endif()
endfunction()

run_round_trip(empty "")
run_round_trip(plain "Huffman CLI integration test: ASCII and punctuation 0123456789")
string(REPEAT "0123456789abcdef" 4097 large_input)
run_round_trip(buffer_boundary "${large_input}")

set(bad_archive "${WORK_DIR}/bad_magic.huf")
file(WRITE "${bad_archive}" "NOT-HUF1")
execute_process(
    COMMAND "${ARCHIVER}" -d "${bad_archive}" "${WORK_DIR}/bad_magic.output"
    RESULT_VARIABLE bad_magic_result
    OUTPUT_QUIET
    ERROR_QUIET
)
if("${bad_magic_result}" STREQUAL "0")
    message(FATAL_ERROR "The CLI accepted an archive with an invalid magic number")
endif()

set(truncated_archive "${WORK_DIR}/truncated.huf")
file(WRITE "${truncated_archive}" "HUF1")
execute_process(
    COMMAND "${ARCHIVER}" -d "${truncated_archive}" "${WORK_DIR}/truncated.output"
    RESULT_VARIABLE truncated_result
    OUTPUT_QUIET
    ERROR_QUIET
)
if("${truncated_result}" STREQUAL "0")
    message(FATAL_ERROR "The CLI accepted a truncated archive header")
endif()
