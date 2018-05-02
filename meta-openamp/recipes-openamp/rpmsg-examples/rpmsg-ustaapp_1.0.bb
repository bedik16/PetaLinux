SUMMARY = "RPMsg examples: Matrix Multiplication demo"


LICENSE = "BSD"
LIC_FILES_CHKSUM = "file://LICENSE;md5=b30cbe0b980e98bfd9759b1e6ba3d107"

SRC_URI = "\
	file://LICENSE \
	file://Makefile \
	file://ustaapp.c \
	"

S = "${WORKDIR}"

DEPENDS = "rpmsg-user-module"

FILES_${PN} = "\
	/usr/bin/ustaapp\
"

do_install () {
	install -d ${D}/usr/bin
	install -m 0755 ustaapp ${D}/usr/bin/ustaapp
}
