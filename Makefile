CC = gcc
TARGET = ext2shell
IMAGE_NAME = "myext2image.img"

# -g informações para debug (gdb)
# -Wall ativa os avisos de código
CFLAGS = -g -Wall

LIBS = -lreadline
SRCS = main.c ext2-impl/ext2-fs-methods.c commands/*.c utils/*.c

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

run: all
	./$(TARGET)

clean:
	rm -f $(TARGET)

image:
	rm $(IMAGE_NAME)
	tar -xf myext2image.tar.gz


# --- Nem sei se vou usar isso ainda ---

generate-ext2:
	dd if=/dev/zero of=./myext2.iso bs=1024 count=64K
	mkfs.ext2 -L "Volume do mouras" -b 1024 ./myext2.iso

verify-ext2:
	e2fsck ${IMAGE_NAME}

mount:
	sudo mount ${IMAGE_NAME} /mnt/ext2-project

umount:
	sudo umount /mnt/ext2-project

copy-files:
	sudo cp -r base-directory/* /mnt/ext2-project/

run-image: generate-ext2 verify-ext2 mount copy-files