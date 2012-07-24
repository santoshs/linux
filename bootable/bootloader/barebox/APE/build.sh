#!/bin/sh

# Build option
 rl_lcd_option="RL_LCD_ENABLE=TRUE";
 fw_lcd_option="FW_LCD_ENABLE=TRUE";
# security_option="INTEGRITY_CHECK_ENABLE=TRUE";
security_option="INTEGRITY_CHECK_ENABLE=FALSE";

all_build()
{
	param="common hw_init usb usb_fb pmic ths std lcd tmu emmc serial disk_drive securelib r_loader flashwriter fastboot high_temp off_charge fota ramset ramset_loader no_battery updater";
	make_func $type $param
}

make_func()
{
	ret=0;
	type=$1;
	command=;
	lcd_option=$rl_lcd_option" "$fw_lcd_option;
	for target in $@
	do
		mkbin="";

		if(test $target = "common") ; then
			cd common/common/build
		elif(test $target = "hw_init") ; then
			cd common/hw_init/build
		elif(test $target = "pmic") ; then
			cd common/pmic/build
		elif(test $target = "ths") ; then
			cd common/ths/build
		elif(test $target = "usb") ; then
			cd common/usb/build
		elif(test $target = "usb_fb") ; then
			cd common/usb/build
		elif(test $target = "std") ; then
			cd common/std/build
		elif(test $target = "lcd") ; then
			cd common/lcd/build
		elif(test $target = "tmu") ; then
			cd common/lcd/build
		elif(test $target = "emmc") ; then
			cd common/flash/emmc/build
		elif(test $target = "fastboot") ; then
			cd fastboot/build
		elif(test $target = "high_temp") ; then
			cd high_temp/build
		elif(test $target = "off_charge") ; then
			cd off_charge/build
		elif(test $target = "fota") ; then
			cd fota/build
		elif(test $target = "disk_drive") ; then
			cd common/disk_drive/build
		elif(test $target = "securelib") ; then
			if(test $security_option = "INTEGRITY_CHECK_ENABLE=TRUE") ; then
				cd securelib/build
			else
				continue;
			fi
		elif(test $target = "r_loader") ; then
			cd r_loader/build
		elif(test $target = "flashwriter") ; then
			cd flashwriter/build
		elif(test $target = "serial") ; then
			cd common/serial/build
		elif(test $target = "ramset") ; then
			cd ramset/build
		elif(test $target = "ramset_loader") ; then
			cd ramset_loader/build
		elif(test $target = "no_battery") ; then
			cd no_battery/build
		elif(test $target = "updater") ; then
			cd flashwriter/build
			if(test $type = "all") ; then
				# Clean object file of flashwriter to rebuild
				if(test -e flw_main.o) ; then
					make -f flashwriter.mk clean
				fi
			else
				if(test $type = "allclean") ; then
					# Remove binary file
					updater_bin="../../bin/updater.bin"
					if(test -e $updater_bin) ; then
						echo rm $updater_bin
						rm $updater_bin
					fi
					
					# Remove stripped file
					updater_stripped="../../bin/updater.stripped.elf"
					if(test -e $updater_stripped) ; then
						echo rm $updater_stripped
						rm $updater_stripped
					fi
				fi
				ret=1;
				continue;
			fi
			target="flashwriter";
			mkbin="DUPDATER=UPDATER";
		else
			continue;
		fi

		# clean flashwriter's object files before re-building
		if(test $target = "flashwriter" -a $type = "all") ; then
			if(test -e flw_main.o); then 
				make -f flashwriter.mk clean
			fi
		fi

		# Build/Clear binary, object files
		command=$target.mk" "$type" "$def" "$mkbin" "$lcd_option" "$security_option;
		make -f $command

		# Create/Delete stripped files
		if(test $type = "all") ; then
			if(test $target = "r_loader" -o $target = "flashwriter" -o $target = "fastboot" -o $target = "ramset" -o $target = "ramset_loader") ; then
				if(test ! -z $mkbin) ; then
					cp flashwriter.axf ../../bin/updater.stripped.elf
					arm-eabi-strip -s ../../bin/updater.stripped.elf
				else				
					cp $target.axf ../../bin/$target.stripped.elf
					arm-eabi-strip -s ../../bin/$target.stripped.elf		
				fi
			fi
		else
			if(test $type = "allclean") ; then
				# For Updater case, it was deleted above
				strippes="../../bin/"$target".stripped.elf"
				if(test -e $strippes) ; then
					echo rm $strippes
					rm $strippes
				fi
			fi
		fi		
		cd -
		ret=1;
	done

	return $ret;
}

#----------------------------
# build start
#----------------------------
type="all";
type_chk="on";
def=;
for arg in $@
do
	case $arg in
		D*)   def=$def" "$arg ;;
	esac

	if(test \( $arg = "allclean" -o $arg = "clean" -o $arg = "all" \) -a $type_chk = "on" ) ; then
		type=$arg;
		type_chk="off";
		continue;
	else
		continue;
	fi
done

make_func $type $@;

if(test $? -eq 0) ; then
	all_build $type
fi


