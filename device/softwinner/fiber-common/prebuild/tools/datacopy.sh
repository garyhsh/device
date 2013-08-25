#!/system/bin/busybox sh

BUSYBOX="/system/bin/busybox"
PSONE_DATA_PATH="/data/data/com.epsxe.ePSXe"
GBA_DATA_PATH="/data/data/com.androidemu.gba"
GENS_DATA_PATH="/data/data/com.androidemu.gens"
ARCADE_DATA_PATH="/data/data/com.tiger.game.arcade2"
N64_DATA_PATH="/data/data/com.androidemu.n64"
GBC_DATA_PATH="/data/data/com.androidemu.gbc"
SNES_DATA_PATH="/data/data/com.androidemu.sneslite"
PSONE_PREBUILD_DATA_PATH="/system/etc/prebuild_data/com.epsxe.ePSXe"
GENS_PREBUILD_DATA_PATH="/system/etc/prebuild_data/com.androidemu.gens"
GBA_PREBUILD_DATA_PATH="/system/etc/prebuild_data/com.androidemu.gba"
ARCADE_PREBUILD_DATA_PATH="/system/etc/prebuild_data/com.tiger.game.arcade2"
N64_PREBUILD_DATA_PATH="/system/etc/prebuild_data/com.androidemu.n64"
GBC_PREBUILD_DATA_PATH="/system/etc/prebuild_data/com.androidemu.gbc"
SNES_PREBUILD_DATA_PATH="/system/etc/prebuild_data/com.androidemu.sneslite"

# ps1
if [ ! -d $PSONE_DATA_PATH ]; then
	echo "create ps data directory"

	mkdir $PSONE_DATA_PATH
	$BUSYBOX chmod 777 $PSONE_DATA_PATH

	mkdir $PSONE_DATA_PATH/shared_prefs
	$BUSYBOX chmod 777 $PSONE_DATA_PATH/shared_prefs

	$BUSYBOX cp $PSONE_PREBUILD_DATA_PATH/shared_prefs/* $PSONE_DATA_PATH/shared_prefs/
	$BUSYBOX chmod 777 $PSONE_DATA_PATH/shared_prefs/*

	mkdir $PSONE_DATA_PATH/lib
	$BUSYBOX chmod 777 $PSONE_DATA_PATH/lib

	$BUSYBOX cp $PSONE_PREBUILD_DATA_PATH/lib/* $PSONE_DATA_PATH/lib/
	$BUSYBOX chmod 777 $PSONE_DATA_PATH/lib/*
else
	echo "copy ps data"	

	#copy share prefereces data
	if [ ! -d $PSONE_DATA_PATH/shared_prefs ];then
		mkdir $PSONE_DATA_PATH/shared_prefs
		$BUSYBOX chmod 777 $PSONE_DATA_PATH/shared_prefs
		
		$BUSYBOX cp $PSONE_PREBUILD_DATA_PATH/shared_prefs/* $PSONE_DATA_PATH/shared_prefs/
		$BUSYBOX chmod 777 $PSONE_DATA_PATH/shared_prefs/*
	else
		$BUSYBOX chmod 777 $PSONE_DATA_PATH/shared_prefs

		if [ ! -e $PSONE_DATA_PATH/shared_prefs/com.epsxe.ePSXe_preferences.xml ];then
			$BUSYBOX cp $PSONE_PREBUILD_DATA_PATH/shared_prefs/* $PSONE_DATA_PATH/shared_prefs/
		fi

		$BUSYBOX chmod 777 $PSONE_DATA_PATH/shared_prefs/*
	fi

	#copy lib
	if [ ! -d $PSONE_DATA_PATH/lib ];then
		mkdir $PSONE_DATA_PATH/lib
		$BUSYBOX chmod 777 $PSONE_DATA_PATH/lib

		$BUSYBOX cp $PSONE_PREBUILD_DATA_PATH/lib/* $PSONE_DATA_PATH/lib/
		$BUSYBOX chmod 755 $PSONE_DATA_PATH/lib/*
	else
		$BUSYBOX chmod 777 $PSONE_DATA_PATH/lib
		
		if [ ! -e $PSONE_DATA_PATH/lib/libdetect.so ];then
			$BUSYBOX cp $PSONE_PREBUILD_DATA_PATH/lib/libdetect.so $PSONE_DATA_PATH/lib/libdetect.so
		fi

		if [ ! -e $PSONE_DATA_PATH/lib/libepsxe.so ];then
			$BUSYBOX cp $PSONE_PREBUILD_DATA_PATH/lib/libepsxe.so $PSONE_DATA_PATH/lib/libepsxe.so
		fi

		if [ ! -e $PSONE_DATA_PATH/lib/libepsxe_tegra2.so ];then
			$BUSYBOX cp $PSONE_PREBUILD_DATA_PATH/lib/libepsxe_tegra2.so $PSONE_DATA_PATH/lib/libepsxe_tegra2.so
		fi

		if [ ! -e $PSONE_DATA_PATH/lib/libgamedetect.so ];then
			$BUSYBOX cp $PSONE_PREBUILD_DATA_PATH/lib/libgamedetect.so $PSONE_DATA_PATH/lib/libgamedetect.so
		fi

		$BUSYBOX chmod 755 $PSONE_DATA_PATH/lib/*
	fi
fi


# gba
if [ ! -d $GBA_DATA_PATH ]; then
	echo "create gba data directory"

	mkdir $GBA_DATA_PATH
	$BUSYBOX chmod 777 $GBA_DATA_PATH

	mkdir $GBA_DATA_PATH/shared_prefs
	$BUSYBOX chmod 777 $GBA_DATA_PATH/shared_prefs

	$BUSYBOX cp $GBA_PREBUILD_DATA_PATH/shared_prefs/* $GBA_DATA_PATH/shared_prefs/
	$BUSYBOX chmod 777 $GBA_DATA_PATH/shared_prefs/*

	mkdir $GBA_DATA_PATH/lib
	$BUSYBOX chmod 777 $GBA_DATA_PATH/lib

	$BUSYBOX cp $GBA_PREBUILD_DATA_PATH/lib/* $GBA_DATA_PATH/lib/
	$BUSYBOX chmod 777 $GBA_DATA_PATH/lib/*
else
	echo "copy gba data"	

	#copy share prefereces data
	if [ ! -d $GBA_DATA_PATH/shared_prefs ];then
		mkdir $GBA_DATA_PATH/shared_prefs
		$BUSYBOX chmod 777 $GBA_DATA_PATH/shared_prefs
		
		$BUSYBOX cp $GBA_PREBUILD_DATA_PATH/shared_prefs/* $GBA_DATA_PATH/shared_prefs/
		$BUSYBOX chmod 777 $GBA_DATA_PATH/shared_prefs/*
	else
		$BUSYBOX chmod 777 $GBA_DATA_PATH/shared_prefs

		if [ ! -e $GBA_DATA_PATH/shared_prefs/com.androidemu.gba_preferences.xml ];then
			$BUSYBOX cp $GBA_PREBUILD_DATA_PATH/shared_prefs/* $GBA_DATA_PATH/shared_prefs/
		fi

		$BUSYBOX chmod 777 $GBA_DATA_PATH/shared_prefs/*
	fi

	#copy lib
	if [ ! -d $GBA_DATA_PATH/lib ];then
		mkdir $GBA_DATA_PATH/lib
		$BUSYBOX chmod 777 $GBA_DATA_PATH/lib

		$BUSYBOX cp $GBA_PREBUILD_DATA_PATH/lib/* $GBA_DATA_PATH/lib/
		$BUSYBOX chmod 755 $GBA_DATA_PATH/lib/*
	else
		$BUSYBOX chmod 777 $GBA_DATA_PATH/lib
		
		if [ ! -e $GBA_DATA_PATH/lib/libemumedia.so ];then
			$BUSYBOX cp $GBA_PREBUILD_DATA_PATH/lib/libemumedia.so $GBA_DATA_PATH/lib/libemumedia.so
		fi

		if [ ! -e $GBA_DATA_PATH/lib/libgba.so ];then
			$BUSYBOX cp $GBA_PREBUILD_DATA_PATH/lib/libgba.so $GBA_DATA_PATH/lib/libgba.so
		fi

		$BUSYBOX chmod 755 $GBA_DATA_PATH/lib/*
	fi
fi

# gens
if [ ! -d $GENS_DATA_PATH ]; then
	echo "create gens data directory"

	mkdir $GENS_DATA_PATH
	$BUSYBOX chmod 777 $GENS_DATA_PATH

	mkdir $GENS_DATA_PATH/shared_prefs
	$BUSYBOX chmod 777 $GENS_DATA_PATH/shared_prefs

	$BUSYBOX cp $GENS_PREBUILD_DATA_PATH/shared_prefs/* $GENS_DATA_PATH/shared_prefs/
	$BUSYBOX chmod 777 $GENS_DATA_PATH/shared_prefs/*

	mkdir $GENS_DATA_PATH/lib
	$BUSYBOX chmod 777 $GENS_DATA_PATH/lib

	$BUSYBOX cp $GENS_PREBUILD_DATA_PATH/lib/* $GENS_DATA_PATH/lib/
	$BUSYBOX chmod 777 $GENS_DATA_PATH/lib/*
else
	echo "copy gens data"

	#copy share prefereces data
	if [ ! -d $GENS_DATA_PATH/shared_prefs ];then
		mkdir $GENS_DATA_PATH/shared_prefs
		$BUSYBOX chmod 777 $GENS_DATA_PATH/shared_prefs
		
		$BUSYBOX cp $GENS_PREBUILD_DATA_PATH/shared_prefs/* $GENS_DATA_PATH/shared_prefs/
		$BUSYBOX chmod 777 $GENS_DATA_PATH/shared_prefs/*
	else
		$BUSYBOX chmod 777 $GENS_DATA_PATH/shared_prefs

		if [ ! -e $GENS_DATA_PATH/shared_prefs/com.androidemu.gens_preferences.xml ];then
			$BUSYBOX cp $GENS_PREBUILD_DATA_PATH/shared_prefs/* $GENS_DATA_PATH/shared_prefs/
		fi

		$BUSYBOX chmod 777 $GENS_DATA_PATH/shared_prefs/*
	fi

	#copy lib
	if [ ! -d $GENS_DATA_PATH/lib ];then
		mkdir $GENS_DATA_PATH/lib
		$BUSYBOX chmod 777 $GENS_DATA_PATH/lib

		$BUSYBOX cp $GENS_PREBUILD_DATA_PATH/lib/* $GENS_DATA_PATH/lib/
		$BUSYBOX chmod 755 $GENS_DATA_PATH/lib/*
	else
		$BUSYBOX chmod 777 $GENS_DATA_PATH/lib
		
		if [ ! -e $GENS_DATA_PATH/lib/libemu.so ];then
			$BUSYBOX cp $GENS_PREBUILD_DATA_PATH/lib/libemu.so $GENS_DATA_PATH/lib/libemu.so
		fi

		if [ ! -e $GENS_DATA_PATH/lib/libemumedia.so ];then
			$BUSYBOX cp $GENS_PREBUILD_DATA_PATH/lib/libemumedia.so $GENS_DATA_PATH/lib/libemumedia.so
		fi

		if [ ! -e $GENS_DATA_PATH/lib/libgens.so ];then
			$BUSYBOX cp $GENS_PREBUILD_DATA_PATH/lib/libgens.so $GENS_DATA_PATH/lib/libgens.so
		fi
		$BUSYBOX chmod 755 $GENS_DATA_PATH/lib/*
	fi
fi


# arcade
if [ ! -d $ARCADE_DATA_PATH ]; then
	echo "create arcade data directory"

	mkdir $ARCADE_DATA_PATH
	$BUSYBOX chmod 777 $ARCADE_DATA_PATH

	mkdir $ARCADE_DATA_PATH/shared_prefs
	$BUSYBOX chmod 777 $ARCADE_DATA_PATH/shared_prefs

	$BUSYBOX cp $ARCADE_PREBUILD_DATA_PATH/shared_prefs/* $ARCADE_DATA_PATH/shared_prefs/
	$BUSYBOX chmod 777 $ARCADE_DATA_PATH/shared_prefs/*

	mkdir $ARCADE_DATA_PATH/lib
	$BUSYBOX chmod 777 $ARCADE_DATA_PATH/lib

	$BUSYBOX cp $ARCADE_PREBUILD_DATA_PATH/lib/* $ARCADE_DATA_PATH/lib/
	$BUSYBOX chmod 777 $ARCADE_DATA_PATH/lib/*
else
	echo "copy arcade data"

	#copy share prefereces data
	if [ ! -d $ARCADE_DATA_PATH/shared_prefs ];then
		mkdir $ARCADE_DATA_PATH/shared_prefs
		$BUSYBOX chmod 777 $ARCADE_DATA_PATH/shared_prefs
		
		$BUSYBOX cp $ARCADE_PREBUILD_DATA_PATH/shared_prefs/* $ARCADE_DATA_PATH/shared_prefs/
		$BUSYBOX chmod 777 $ARCADE_DATA_PATH/shared_prefs/*
	else
		$BUSYBOX chmod 777 $ARCADE_DATA_PATH/shared_prefs

		if [ ! -e $ARCADE_DATA_PATH/shared_prefs/com.tiger.game.arcade2_preferences.xml ];then
			$BUSYBOX cp $ARCADE_PREBUILD_DATA_PATH/shared_prefs/* $ARCADE_DATA_PATH/shared_prefs/
		fi

		$BUSYBOX chmod 777 $ARCADE_DATA_PATH/shared_prefs/*
	fi

	#copy lib
	if [ ! -d $ARCADE_DATA_PATH/lib ];then
		mkdir $ARCADE_DATA_PATH/lib
		$BUSYBOX chmod 777 $ARCADE_DATA_PATH/lib

		$BUSYBOX cp $ARCADE_PREBUILD_DATA_PATH/lib/* $ARCADE_DATA_PATH/lib/
		$BUSYBOX chmod 755 $ARCADE_DATA_PATH/lib/*
	else
		$BUSYBOX chmod 777 $ARCADE_DATA_PATH/lib
		
		if [ ! -e $ARCADE_DATA_PATH/lib/libtigercore.so ];then
			$BUSYBOX cp $ARCADE_PREBUILD_DATA_PATH/lib/libtigercore.so $ARCADE_DATA_PATH/lib/libtigercore.so
		fi

		if [ ! -e $ARCADE_DATA_PATH/lib/libtigerfba.so ];then
			$BUSYBOX cp $ARCADE_PREBUILD_DATA_PATH/lib/libtigerfba.so $ARCADE_DATA_PATH/lib/libtigerfba.so
		fi

		$BUSYBOX chmod 755 $ARCADE_DATA_PATH/lib/*
	fi
fi



# n64
if [ ! -d $N64_DATA_PATH ]; then
        echo "create n64 data directory"

        mkdir $N64_DATA_PATH
        $BUSYBOX chmod 777 $N64_DATA_PATH

        mkdir $N64_DATA_PATH/shared_prefs
        $BUSYBOX chmod 777 $N64_DATA_PATH/shared_prefs

        $BUSYBOX cp $N64_PREBUILD_DATA_PATH/shared_prefs/* $N64_DATA_PATH/shared_prefs/
        $BUSYBOX chmod 777 $N64_DATA_PATH/shared_prefs/*

        mkdir $N64_DATA_PATH/lib
        $BUSYBOX chmod 777 $N64_DATA_PATH/lib

        $BUSYBOX cp $N64_PREBUILD_DATA_PATH/lib/* $N64_DATA_PATH/lib/
        $BUSYBOX chmod 777 $N64_DATA_PATH/lib/*
else
        echo "copy n64 data"

        #copy share prefereces data
        if [ ! -d $N64_DATA_PATH/shared_prefs ];then
                mkdir $N64_DATA_PATH/shared_prefs
                $BUSYBOX chmod 777 $N64_DATA_PATH/shared_prefs

                $BUSYBOX cp $N64_PREBUILD_DATA_PATH/shared_prefs/* $N64_DATA_PATH/shared_prefs/
                $BUSYBOX chmod 777 $N64_DATA_PATH/shared_prefs/*
        else
                $BUSYBOX chmod 777 $N64_DATA_PATH/shared_prefs

                if [ ! -e $N64_DATA_PATH/shared_prefs/com.androidemu.n64_preferences.xml ];then
                        $BUSYBOX cp $N64_PREBUILD_DATA_PATH/shared_prefs/* $N64_DATA_PATH/shared_prefs/
                fi

                $BUSYBOX chmod 777 $N64_DATA_PATH/shared_prefs/*
        fi

        #copy lib
        if [ ! -d $N64_DATA_PATH/lib ];then
                mkdir $N64_DATA_PATH/lib
                $BUSYBOX chmod 777 $N64_DATA_PATH/lib

                $BUSYBOX cp $N64_PREBUILD_DATA_PATH/lib/* $N64_DATA_PATH/lib/
                $BUSYBOX chmod 755 $N64_DATA_PATH/lib/*
        else
                $BUSYBOX chmod 777 $N64_DATA_PATH/lib

                if [ ! -e $N64_DATA_PATH/lib/libmupen64plus-core.so ];then
                        $BUSYBOX cp $N64_PREBUILD_DATA_PATH/lib/libmupen64plus-core.so $N64_DATA_PATH/lib/libmupen64plus-core.so
                fi

                if [ ! -e $N64_DATA_PATH/lib/libmupen64plus-rsp-hle.so ];then
                        $BUSYBOX cp $N64_PREBUILD_DATA_PATH/lib/libmupen64plus-rsp-hle.so $N64_DATA_PATH/lib/libmupen64plus-rsp-hle.so
                fi

                if [ ! -e $N64_DATA_PATH/lib/libn64.so ];then
                        $BUSYBOX cp $N64_PREBUILD_DATA_PATH/lib/libn64.so $N64_DATA_PATH/lib/libn64.so
                fi

                if [ ! -e $N64_DATA_PATH/lib/libn64-audio.so ];then
                        $BUSYBOX cp $N64_PREBUILD_DATA_PATH/lib/libn64-audio.so $N64_DATA_PATH/lib/libn64-audio.so
                fi

                if [ ! -e $N64_DATA_PATH/lib/libn64-input.so ];then
                        $BUSYBOX cp $N64_PREBUILD_DATA_PATH/lib/libn64-input.so $N64_DATA_PATH/lib/libn64-input.so
                fi

                if [ ! -e $N64_DATA_PATH/lib/libn64-video.so ];then
                        $BUSYBOX cp $N64_PREBUILD_DATA_PATH/lib/libn64-video.so $N64_DATA_PATH/lib/libn64-video.so
                fi

                $BUSYBOX chmod 755 $N64_DATA_PATH/lib/*
        fi
fi



# gbc
if [ ! -d $GBC_DATA_PATH ]; then
        echo "create gbc data directory"

        mkdir $GBC_DATA_PATH
        $BUSYBOX chmod 777 $GBC_DATA_PATH

        mkdir $GBC_DATA_PATH/shared_prefs
        $BUSYBOX chmod 777 $GBC_DATA_PATH/shared_prefs

        $BUSYBOX cp $GBC_PREBUILD_DATA_PATH/shared_prefs/* $GBC_DATA_PATH/shared_prefs/
        $BUSYBOX chmod 777 $GBC_DATA_PATH/shared_prefs/*

        mkdir $GBC_DATA_PATH/lib
        $BUSYBOX chmod 777 $GBC_DATA_PATH/lib

        $BUSYBOX cp $GBC_PREBUILD_DATA_PATH/lib/* $GBC_DATA_PATH/lib/
        $BUSYBOX chmod 777 $GBC_DATA_PATH/lib/*
else
        echo "copy gbc data"

        #copy share prefereces data
        if [ ! -d $GBC_DATA_PATH/shared_prefs ];then
                mkdir $GBC_DATA_PATH/shared_prefs
                $BUSYBOX chmod 777 $GBC_DATA_PATH/shared_prefs

                $BUSYBOX cp $GBC_PREBUILD_DATA_PATH/shared_prefs/* $GBC_DATA_PATH/shared_prefs/
                $BUSYBOX chmod 777 $GBC_DATA_PATH/shared_prefs/*
        else
                $BUSYBOX chmod 777 $GBC_DATA_PATH/shared_prefs

                if [ ! -e $GBC_DATA_PATH/shared_prefs/com.androidemu.gbc_preferences.xml ];then
                        $BUSYBOX cp $GBC_PREBUILD_DATA_PATH/shared_prefs/* $GBC_DATA_PATH/shared_prefs/
                fi

                $BUSYBOX chmod 777 $GBC_DATA_PATH/shared_prefs/*
        fi

        #copy lib
        if [ ! -d $GBC_DATA_PATH/lib ];then
                mkdir $GBC_DATA_PATH/lib
                $BUSYBOX chmod 777 $GBC_DATA_PATH/lib

                $BUSYBOX cp $GBC_PREBUILD_DATA_PATH/lib/* $GBC_DATA_PATH/lib/
                $BUSYBOX chmod 755 $GBC_DATA_PATH/lib/*
        else
                $BUSYBOX chmod 777 $GBC_DATA_PATH/lib

                if [ ! -e $GBC_DATA_PATH/lib/libemu.so ];then
                        $BUSYBOX cp $GBC_PREBUILD_DATA_PATH/lib/libemu.so $GBC_DATA_PATH/lib/libemu.so
                fi

                if [ ! -e $GBC_DATA_PATH/lib/libemumedia.so ];then
                        $BUSYBOX cp $GBC_PREBUILD_DATA_PATH/lib/libemumedia.so $GBC_DATA_PATH/lib/libemumedia.so
                fi

                if [ ! -e $GBC_DATA_PATH/lib/libgbc.so ];then
                        $BUSYBOX cp $GBC_PREBUILD_DATA_PATH/lib/libgbc.so $GBC_DATA_PATH/lib/libgbc.so
                fi

                $BUSYBOX chmod 755 $GBC_DATA_PATH/lib/*
        fi
fi


# snes
if [ ! -d $SNES_DATA_PATH ]; then
        echo "create snes data directory"

        mkdir $SNES_DATA_PATH
        $BUSYBOX chmod 777 $SNES_DATA_PATH

        mkdir $SNES_DATA_PATH/shared_prefs
        $BUSYBOX chmod 777 $SNES_DATA_PATH/shared_prefs

        $BUSYBOX cp $SNES_PREBUILD_DATA_PATH/shared_prefs/* $SNES_DATA_PATH/shared_prefs/
        $BUSYBOX chmod 777 $SNES_DATA_PATH/shared_prefs/*

        mkdir $SNES_DATA_PATH/lib
        $BUSYBOX chmod 777 $SNES_DATA_PATH/lib

        $BUSYBOX cp $SNES_PREBUILD_DATA_PATH/lib/* $SNES_DATA_PATH/lib/
        $BUSYBOX chmod 777 $SNES_DATA_PATH/lib/*
else
        echo "copy snes data"

        #copy share prefereces data
        if [ ! -d $SNES_DATA_PATH/shared_prefs ];then
                mkdir $SNES_DATA_PATH/shared_prefs
                $BUSYBOX chmod 777 $SNES_DATA_PATH/shared_prefs

                $BUSYBOX cp $SNES_PREBUILD_DATA_PATH/shared_prefs/* $SNES_DATA_PATH/shared_prefs/
                $BUSYBOX chmod 777 $SNES_DATA_PATH/shared_prefs/*
        else
                $BUSYBOX chmod 777 $SNES_DATA_PATH/shared_prefs

                if [ ! -e $SNES_DATA_PATH/shared_prefs/com.androidemu.senslite_preferences.xml ];then
                        $BUSYBOX cp $SNES_PREBUILD_DATA_PATH/shared_prefs/* $SNES_DATA_PATH/shared_prefs/
                fi

                $BUSYBOX chmod 777 $SNES_DATA_PATH/shared_prefs/*
        fi

        #copy lib
        if [ ! -d $SNES_DATA_PATH/lib ];then
                mkdir $SNES_DATA_PATH/lib
                $BUSYBOX chmod 777 $SNES_DATA_PATH/lib

                $BUSYBOX cp $SNES_PREBUILD_DATA_PATH/lib/* $SNES_DATA_PATH/lib/
                $BUSYBOX chmod 755 $SNES_DATA_PATH/lib/*
        else
                $BUSYBOX chmod 777 $SNES_DATA_PATH/lib

                if [ ! -e $SNES_DATA_PATH/lib/libemu.so ];then
                        $BUSYBOX cp $SNES_PREBUILD_DATA_PATH/lib/libemu.so $SNES_DATA_PATH/lib/libemu.so
                fi

                if [ ! -e $SNES_DATA_PATH/lib/libemumedia.so ];then
                        $BUSYBOX cp $SNES_PREBUILD_DATA_PATH/lib/libemumedia.so $SNES_DATA_PATH/lib/libemumedia.so
                fi

                if [ ! -e $SNES_DATA_PATH/lib/libsnes.so ];then
                        $BUSYBOX cp $SNES_PREBUILD_DATA_PATH/lib/libsnes.so $SNES_DATA_PATH/lib/libsnes.so
                fi

                if [ ! -e $SNES_DATA_PATH/lib/libsnes_comp.so ];then
                        $BUSYBOX cp $SNES_PREBUILD_DATA_PATH/lib/libsnes_comp.so $SNES_DATA_PATH/lib/libsnes_comp.so
                fi

                $BUSYBOX chmod 755 $SNES_DATA_PATH/lib/*
        fi
fi
