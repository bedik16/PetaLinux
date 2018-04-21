# Petalinux AMP Design with Linux on cpu0 and FreeRTOS on cpu1
This projects aims to build an AMP design using Xilinx ZedBoard FPGA to implement multi-agent decision mechanism on linux and accessing low-level hardware via FreeRTOS using shared memory space on board.

**Installation:**

You somehow decided to work with an AMP design using Xilinx Zedboard, well congratulations... You will need 2 things in order to walk on this road. Caffeine and more caffeine. So grab your favourite caffeine based poison and get ready for a mental ride on a buggy road. And let me make a thing clear for you. NEVER fully trust a Xilinx let's get started manual, it's full of bugs and mistakes...  

1. We have to install Vivado SDK and Design Suite so download it from official website and open up your terminal

         user@linuxworkstation ~ $ cd Downloads
	 user@linuxworkstation ~ $ chmod +x VivadoInstaller
	 user@linuxworkstation ~ $ ./VivadoInstaller

2. You will get an error, because you have no permit to create /opt/Xilinx, here is the solution

         user@linuxworkstation ~ $ mkdir /opt/Xilinx
	 user@linuxworkstation ~ $ chown <user-name> /opt/Xilinx
	 user@linuxworkstation ~ $ chmod u+rwx /opt/Xilinx

3. Install dependencies on UG1144, page 9 (Don't worry if you miss some of those dependencies, installer will let you know which packages are missing anyway)

4. Download Petalinux

         user@linuxworkstation ~ $ cd Downloads
         user@linuxworkstation ~ $ chmod +x <installer>

5. If you get an error due to openssl on the previous step, try

         user@linuxworkstation ~ $ apt-cache search libssl | grep ssl

and install all the results

6. Now set your shell bash

         user@linuxworkstation ~ $ chsh -s /bin/bash
	 user@linuxworkstation ~ $ sudo rm /bin/bash
	 user@linuxworkstation ~ $ sudo ln -s /bin/bash /bin/sh

7. Congrats, you successfully installed Petalinux, now go to the petalinux folder and source the settings each time you want to build a kernel image with petalinux tools

         user@linuxworkstation ~ $ cd /path/to/your/petalinux/folder
	 user@linuxworkstation ~ $ source settings.sh

Verify by the command below, if the result is your petalinux folder then you are good to go;

         user@linuxworkstation ~ $ echo $PETALINUX

8. We are ready to build our AMP project using petalinux tools. However, we still need to download the board support package for ZedBoard from Xilinx website

9. Change into directory you would like to create your projects under;

         user@linuxworkstation ~ $ cd /path/to/create/your/project
	 user@linuxworkstation ~ $ petalinux-create -t project -n <name of the project> -s <path to bsp>

10. After successfully creating our project, we have to launch Xilinx SDK in order to create our remote application, open up a new terminal;

         user@linuxworkstation ~ $ sudo xsdk

11. From the Xilinx SDK window, create the application project by selecting File > New > Application Projects.

a. Specify the BSP OS platform:
- freertos<version>_xilinx for a FreeRTOS application.

b. Specify the Hardware Platform:
-Select ZedBoardHW or something similar to that from the available options.

c. Select the Processor:
-Select ps7_cortexa9_1.

d. Select, create new BSP.

e. Click Next to select an available template (do not click Finish).

12. Select OpenAMP matrix multiplication Demo as the template.

13. Click Finish.

14. In the Xilinx SDK project explorer, right-click the BSP and select Board Support Package Settings.

15. Navigate to the BSP Settings > Overview > OpenAMP.

16. Set the WITH_PROXY parameter as false.

17. Navigate to the BSP settings drivers: Settings > Overview > Drivers > <selected_processor>.

18. Add -DUSE_AMP=1 to the extra_compiler_flags (You might have to extend to view with your mouse in order to write into empty field).

19. Click the OK button.

20. Edit ps7_ddr_0_S_AXI_BASEADDR Base Address as 0x1e000000 instead of 0x3e000000 (Xilinx manual claims 0x3e000000 as the right address but Zedboard has only 512 Mb of RAM and if you don't fix this step, AMP solution will give an unresolved pointer error. This bug costed me 2 weeks of time and time is money !!!)

21. Open up rsc_table.c from your project sources and edit all the addresses starts with 0x3e..... into 0x1e..... (for example #define RING_TX  0x1e600000)

22. Build the project and you will get the final .elf file under your workspace. This will be the executable firmware for your remote core.

23. Now go back to the petalinux project that you created and create an app with the following command

         user@linuxworkstation ~ $ petalinux-create -t apps --template install -n <app_name> --enable

24. Copy the firmware (that is, the .elf file) built with Xilinx SDK for the remote processor into this directory and delete the existing template in that folder:

         project-spec/meta-user/recipes-apps/<app-name>/files/

25. Modify the project-spec/meta-user/recipes-apps/<app_name>/<app_name>.bb to install the remote processor firmware in the RootFS. (In my case as you can see below, I named my firmware into matrix.elf

This file is the matrix.elf recipe.

SUMMARY = "Simple matrix.elf application"
SECTION = "PETALINUX/apps"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

SRC_URI = "file://matrix.elf \
	"

S = "${WORKDIR}"

INSANE_SKIP_${PN} = "arch"

do_install() {
	     install -d ${D}/lib/firmware
	     install -m 0755 ${S}/matrix.elf ${D}/lib/firmware/matrix.elf
}

FILES_${PN} = "/lib/firmware/matrix.elf"
	 

	"

S = "${WORKDIR}"

INSANE_SKIP_${PN} = "arch"

do_install() {
	     install -d ${D}/lib/firmware
	     install -m 0755 ${S}/matrix.elf ${D}/lib/firmware/matrix.elf
}

FILES_${PN} = "/lib/firmware/matrix.elf"
	 
26. Configure the kernel options to work with OpenAMP. 

a. Start the PetaLinux Kernel configuration tool:

         user@linuxworkstation ~ $ petalinux-config -c kernel 

b. Enable loadable module support:

[*] Enable loadable module support --->

c. Enable user space firmware loading support:

Device Drivers --->
Generic Driver Options --->
<*> Userspace firmware loading support

d. Enable the remoteproc driver support: 

Device Drivers --->
Remoteproc drivers --->
<M> Support ZYNQ remoteproc

27. Enable all of the modules and applications in the RootFS:

a. Open the RootFS configuration menu:

         user@linuxworkstation ~ $ petalinux-config -c rootfs

b. Ensure the OpenAMP applications and rpmsg modules are enabled:

Filesystem Packages --->
misc --->
packagegroup-petalinux-openamp --->
[*] packagegroup-petalinux-openamp

28. Exit the rootfs configuration and we are ready to edit our device tree overlay. For this change your directory into; 

         user@linuxworkstation ~ $ cd /path/to/your/project/project-spec/meta-user/recipes-bsp/device-tree/files/openamp-overlay.dtsi

29. Edit the device tree overlay as follows:

/ {
	reserved-memory {
		#address-cells = <1>;
		#size-cells = <1>;
		ranges;
		rproc_0_reserved: rproc@3e000000 {
			no-map;
			reg =  <0x3e000000 0x01000000>;
		};
	};

	amba {
		elf_ddr_0: ddr@0 {
			compatible = "mmio-sram";
			reg = <0x3e000000 0x400000>;
		};
	};
	remoteproc0: remoteproc@0 {
		compatible = "xlnx,zynq_remoteproc";
		firmware = "firmware";
		vring0 = <15>;
		vring1 = <14>;
		srams = <&elf_ddr_0>;

	};
};

30. We are ready to build our application !!! Change in to your project directory and build your project as follows 

         user@linuxworkstation ~ $ petalinux-build

31. If everything goes according to the plan you should be able to build project successfully, now next step is to create bootloader for the SD Card. Go to /images/linux directory. 

         user@linuxworkstation ~ $ petalinux-package --boot --fsbl <FSBL IMAGE> --fpga <FPGA BITSTREAM> --u-boot 

(FSBL image is the <zynq_fsbl.elf> file and FPGA bitstream is the <download.bit> file)

32. Everything is ready for the boot on the hardware. Now we need an SD card of the minimum size of 4 GB in order to load our linux system. To make the proper partioning, we need a tool such as GPARTED. Install GPARTED and configure the SD card into 2 sections. First section must be aliased as BOOT and should be 1024 MB of size and FAT32 file type. Make sure to leave 4 MB of preceding place before the first partition. Second partitition must be aliased as rootfs and should have the 4 MB of preceding size and should take all of the remaining space on the SD Card. After our SD card is ready copy image.ub and BOOT.bin into your BOOT part of the partition. 

         user@linuxworkstation ~ $ cp BOOT.bin /media/<username>/BOOT 
         user@linuxworkstation ~ $ cp image.ub /media/<username>/BOOT

33. Copy rootfs.cpio, rootfs.cpio.gz and rootfs.tar.gz into your rootfs partition

         user@linuxworkstation ~ $ sudo cp rootfs.cpio /media/<username>/rootfs
         user@linuxworkstation ~ $ sudo cp rootfs.cpio.gz /media/<username>/rootfs
         user@linuxworkstation ~ $ sudo cp rootfs.tar.gz /media/<username>/rootfs

34. Untar the tar file under your rootfs partition

         user@linuxworkstation ~ $ sudo tar xvf rootfs.tar.gz 

35. We are good to go !!! Plug your SD card into ZedBoard and plug your USB cable into UART port of the board. Open up a terminal (I preffer GTKTERM) and set the baudrate into 115200. After board gets ready type "run bootcmd", the board should start booting up and a login screen should appear. Login with the user name as root and password as root. 

 
