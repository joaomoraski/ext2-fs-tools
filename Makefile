# Generate the image 
# 
generate-ext2:
	dd if=/dev/zero of=./myext2.iso bs=1024 count=64K
	mkfs.ext2 -b 1024 ./myext2.iso

verify-ext2:
	e2fsck myext2.iso

mount:
	sudo mount myext2.iso /mnt/ext2-project

umount:
	sudo umount /mnt/ext2-project

copy-files:
	sudo cp -r base-directory/* /mnt/ext2-project/

run-image: generate-ext2 verify-ext2 mount copy-files

run:
	gcc main.c -o main -lext2fs
	./main

clean:
	rm -f main *.img *.iso