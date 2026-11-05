# fmt --check 는 파일을 고치지 않고 포맷이 필요하면 종료 코드 1 로 끝난다.
# 포맷한 뒤의 --check 는 0 이다. 여러 파일을 한 번에 받는다.
# 사용: cmake -DKINGSEJONG=<바이너리> -DSOURCE=<포맷 안 된 파일> -DWORK=<작업 디렉터리> -P fmt_check.cmake
file(MAKE_DIRECTORY "${WORK}")
configure_file("${SOURCE}" "${WORK}/a.ksj" COPYONLY)
configure_file("${SOURCE}" "${WORK}/b.ksj" COPYONLY)
file(SHA256 "${SOURCE}" original)

execute_process(COMMAND "${KINGSEJONG}" fmt --check "${WORK}/a.ksj" "${WORK}/b.ksj"
    RESULT_VARIABLE rc OUTPUT_VARIABLE out)
file(SHA256 "${WORK}/a.ksj" after)
if(NOT rc EQUAL 1 OR NOT after STREQUAL original OR NOT out MATCHES "a\\.ksj.*b\\.ksj")
    message(FATAL_ERROR "--check on unformatted files: rc=${rc}, changed=${after}, out=${out}")
endif()

execute_process(COMMAND "${KINGSEJONG}" fmt "${WORK}/a.ksj" "${WORK}/b.ksj" RESULT_VARIABLE rc)
file(SHA256 "${WORK}/b.ksj" after)
if(NOT rc EQUAL 0 OR after STREQUAL original)
    message(FATAL_ERROR "fmt did not format both files: rc=${rc}")
endif()

execute_process(COMMAND "${KINGSEJONG}" fmt --check "${WORK}/a.ksj" "${WORK}/b.ksj" RESULT_VARIABLE rc)
if(NOT rc EQUAL 0)
    message(FATAL_ERROR "--check after fmt: rc=${rc}")
endif()

execute_process(COMMAND "${KINGSEJONG}" fmt --chek "${WORK}/a.ksj" RESULT_VARIABLE rc ERROR_VARIABLE err)
if(NOT rc EQUAL 1 OR NOT err MATCHES "알 수 없는 옵션")
    message(FATAL_ERROR "unknown option was not rejected: rc=${rc}, err=${err}")
endif()
