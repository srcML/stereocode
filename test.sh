STEREOCODE=$1
TEST_FILE=$2

rm -f $TEST_FILE.annotated.xml

$STEREOCODE -a $TEST_FILE
diff $TEST_FILE.BASE.xml $TEST_FILE.annotated.xml
