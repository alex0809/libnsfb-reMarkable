#!/bin/sh

TEST_PATH=$1

${TEST_PATH}/test_frontend
${TEST_PATH}/test_plottest
${TEST_PATH}/test_bezier
${TEST_PATH}/test_path
${TEST_PATH}/test_polygon
${TEST_PATH}/test_polystar

