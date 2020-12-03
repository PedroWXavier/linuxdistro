#Compile the syscall_test.c
BUILDROOT_DIR=$BASE_DIR/..
COMPILER=$BUILDROOT_DIR/output/host/bin/i686-buildroot-linux-uclibc-gcc
$COMPILER -o $BUILDROOT_DIR/output/target/bin/syscall_test $BUILDROOT_DIR/custom-scripts/syscall_test.c

#Compile the trabalho3LabSisOp.c
BUILDROOT_DIR=$BASE_DIR/..
COMPILER=$BUILDROOT_DIR/output/host/bin/i686-buildroot-linux-uclibc-gcc
$COMPILER -o $BUILDROOT_DIR/output/target/bin/trabalho3LabSisOp $BUILDROOT_DIR/custom-scripts/trabalho3LabSisOp.c


