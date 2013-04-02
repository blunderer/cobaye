# *.res files are automatically included by buildsystem

# to add a blob resource: declare the file in RES variable
RES += resources/text.txt

# to customize CFLAGS for a test, append the test name to CFLAGS variable
CFLAGS_dummy = -DCUSTOM_FLAG

# to customize generic flags 
#LDFLAGS += -L<path to your lib dir>
#CFLAGS += -I<path to your include dir>

