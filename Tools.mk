# Tools.mk

# wslbash
# $1 : command to run in WSL
wslbash = MSYS2_ARG_CONV_EXCL=* wsl -d Ubuntu-24.04 -- bash -c "$(1)"

# msys2wsl
# $1 : path to convert
msys2wsl = $(shell echo $1 | sed -E 's:^/([a-zA-Z])/:/mnt/\1/:')

# bin2o
# $1: input file
# $2: output include file
# $3: output obj file
# $4: output assembly file
define bin2o
    @mkdir -p $(dir $2)
    @mkdir -p $(dir $3)
    @mkdir -p $(dir $4)

    @bin2s -a 32 -H $2 $1 > $4
    @$(CCompiler) -x assembler-with-cpp $(CPPFLAGS) $(ASFLAGS) -c $4 -o $3
endef

# glsl2gsh
# $1: compiler path
# $2: input vertex file
# $3: input fragment file
# $4: output file
define glsl2gsh
    @mkdir -p $(dir $4)
    @$(call wslbash, $(call msys2wsl,$1) -vs $(call msys2wsl,$2) -ps $(call msys2wsl,$3) -o $(call msys2wsl,$4))
endef

# cpp2o
# $1: input file
# $2: output file
# $3: dependence file
# $4: flags
define cpp2o
    @mkdir -p $(dir $2)
    @mkdir -p $(dir $3)
    @$(CppCompiler) -c $1 -o $2 -MMD -MP -MF $3 $4
endef

# o2elf
# $1: input files
# $2: output file
# $3: linker flags
# $4: library directory flags
# $5: library link flags
# $6: map file
define o2elf
    @mkdir -p $(dir $2)
    @$(Linker) $1 -o $2 $3 $4 $5 -Wl,-Map,$6
endef

# elf2lst
# $1: input file
# $2: output file
define elf2lst
    @mkdir -p $(dir $2)
    @$(NameList) -CSn $1 > $2
endef

# elf2wps
# $1: input file
# $2: output file
define elf2wps
    @mkdir -p $(dir $2)
	@elf2rpl $1 $2
	@echo 'PL' | dd of=$2 bs=1 seek=9 count=2 conv=notrunc status=none
endef

# o2a
# $1: input files
# $2: output file
define o2a
    @mkdir -p $(dir $2)
	@$(Archive) -rcs $2 $1
endef

# abs2rel
# $1: input absolute pathes
# $2: base directory
define abs2rel
    $(if $(strip $1),$(shell realpath --relative-to=$2 $1))
endef