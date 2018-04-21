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

#
# This file is the matrix.elf recipe.
#

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
	 
