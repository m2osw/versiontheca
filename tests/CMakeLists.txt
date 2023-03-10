# Copyright (c) 2023  Made to Order Software Corp.  All Rights Reserved
#
# https://snapwebsites.org/project/versiontheca
# contact@m2osw.com
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

find_package(SnapCatch2)
if(SnapCatch2_FOUND)

    project(unittest)

    add_executable(${PROJECT_NAME}
        catch_main.cpp

        catch_basic.cpp
        catch_debian.cpp
        catch_decimal.cpp
        catch_part.cpp
        catch_roman.cpp
        catch_rpm.cpp
        catch_unicode.cpp
        catch_version.cpp
    )
    target_include_directories(${PROJECT_NAME}
        PUBLIC
            ${PROJECT_BINARY_DIR}
    )
    target_link_libraries(${PROJECT_NAME}
        versiontheca
        ${SNAPCATCH2_LIBRARIES}
    )


    ##
    ## Run all the tests
    ##
    project(run_versiontheca_tests)
    add_custom_target(run_versiontheca_tests
        COMMAND echo "*** run_versiontheca_tests ***"
        COMMAND unittest
        DEPENDS unittest
    )

else(SnapCatch2_FOUND)

    message("SnapCatch2 not found... no tests will be built.")

endif(SnapCatch2_FOUND)

# vim: ts=4 sw=4 et
