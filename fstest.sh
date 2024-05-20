#Store truth vars
file_paths=(
    "fs" "fs/dir" "fs/dir/file1" "fs/dir/file2" "fs/dir/file3" \
    "fs/dir1" "fs/dir1/long-file-name" "fs/dir1/subdir" "fs/file.1" \
    "fs/dir-with-long-name" "fs/dir-with-long-name/2nd-file-with-long-name" \
    "fs/dir2" "fs/dir2/twenty-seven-character-name" "fs/dir2/twenty-six--character-name" \
    "fs/file.2"
)

#files to paths
declare -A file_to_path
file_to_path["file.1"]="fs/file.1"
file_to_path["file.2"]="fs/file.2"
file_to_path["long-file-name"]="fs/dir1/long-file-name"
file_to_path["twenty-seven-character-name"]="fs/dir2/twenty-seven-character-name"
file_to_path["twenty-six--character-name"]="fs/dir2/twenty-six--character-name"
file_to_path["file1"]="fs/dir/file1"
file_to_path["file2"]="fs/dir/file2"
file_to_path["file3"]="fs/dir/file3"
file_to_path["2nd-file-with-long-name"]="fs/dir-with-long-name/2nd-file-with-long-name"


keys=("file.1" "file.2" "long-file-name" "twenty-seven-character-name" "twenty-six--character-name" \
        "file1" "file2" "file3" "2nd-file-with-long-name")

#Directory contents
fs_contents=("dir" "dir1" "dir2" "dir-with-long-name" "file.1" "file.2")
fs_dir_contents=("file1" "file2" "file3")
fs_dir1_contents=("long-file-name" "subdir")
fs_dir1_subdir_contents=()
fs_dir_with_a_long_name_contents=("2nd-file-with-long-name")
fs_dir2_contents=("twenty-six--character-name" "twenty-seven-character-name")

#File modes and stats
fs=("41ff 4096")
fs_dir=("41ff 4096")
fs_dir_file1=("81ff 100")
fs_dir_file2=("81ed 1200")
fs_dir_file3=("81ff 10111")
fs_dir1=("41ed 4096")
fs_dir1_long_file_name=("81ff 1025")
fs_dir1_subdir=("41ed 4096")
fs_file_1=("81ff 900")
fs_dir_with_long_name=("41ed 4096")
fs_dir_with_long_name_2nd_file_with_long_name=("81ff 200")
fs_dir2=("41ed 4096")
fs_dir2_twenty_seven_character_name=("81ff 100")
fs_dir2_twenty_six_character_name=("81ff 100")
fs_file2=("81a0 2000")

#Checksums
# # file cksum
# fs_dir_file1_cksum=("3405207784 100")
# fs_dir_file2_cksum=("2363425951 1200")
# fs_dir_file3_cksum=("4010891928 10111")
# fs_dir1_long_file_name_cksum=("2473152292 1025")
# fs_file_1_cksum=("3660301625 900")
# fs_dir_with_long_name_2nd_file_with_long_name_cksum=("1100315716 200")
# fs_dir2_twenty_seven_character_name_cksum=("3615276204 100")
# fs_dir2_twenty_six_character_name_cksum=("1609569549 100")
# fs_file_2_cksum=("80949033 2000")

fs_cksum=(
    "2178593158 900"
    "3891739079 2000"
    "2928220301 1025"
    "3761062583 100"
    "3611217367 100"
    "2893082884 100"
    "327293107 1200"
    "926221923 10111"
    "558398486 200"
)

# Combine all directory contents arrays into one array
fs_dirs=("${fs_contents[@]}" "${fs_dir_contents[@]}" "${fs_dir1_contents[@]}" "${fs_dir1_subdir_contents[@]}" "${fs_dir_with_a_long_name_contents[@]}" "${fs_dir2_contents[@]}")

# Combine all stat values into an array
fs_stats=("${fs[@]}" "${fs_dir[@]}" "${fs_dir_file1[@]}" "${fs_dir_file2[@]}" "${fs_dir_file3[@]}" \
          "${fs_dir1[@]}" "${fs_dir1_long_file_name[@]}" "${fs_dir1_subdir[@]}" "${fs_file_1[@]}" \
          "${fs_dir_with_long_name[@]}" "${fs_dir_with_long_name_2nd_file_with_long_name[@]}" \
          "${fs_dir2[@]}" "${fs_dir2_twenty_seven_character_name[@]}" "${fs_dir2_twenty_six_character_name[@]}" \
          "${fs_file2[@]}"
)

# # Combine all cksum values into an array
# fs_cksum=("$fs_dir_file1_cksum[@]" "$fs_dir_file2_cksum[@]" "$fs_dir_file3_cksum[@]" \
#             "$fs_dir1_long_file_name_cksum[@]" "$fs_file_1_cksum[@]" "$fs_dir_with_long_name_2nd_file_with_long_name_cksum[@]" \
#             "$fs_dir2_twenty_seven_character_name_cksum[@]" "$fs_dir2_twenty_six_character_name_cksum[@]" \
#             "$fs_file_2_cksum[@]")

# Function to test directory contents and file modes
testReadDirs() {
    # Define directory contents using command substitution
    fs_bug_contents=($(ls fs))
    fs_dir_bug_contents=($(ls fs/dir))
    fs_dir1_bug_contents=($(ls fs/dir1))
    fs_dir_1_subdir_bug_contents=($(ls fs/dir1/subdir))
    fs_dir_with_a_long_name_bug_contents=($(ls fs/dir-with-long-name))
    fs_dir2_bug_contents=($(ls fs/dir2))

    # Combine all directory contents arrays into one array
    bug_dirs=("${fs_bug_contents[@]}" "${fs_dir_bug_contents[@]}" "${fs_dir1_bug_contents[@]}" "${fs_dir_1_subdir_bug_contents[@]}" "${fs_dir_with_a_long_name_bug_contents[@]}" "${fs_dir2_bug_contents[@]}")

    # Check if any directory is missing
    for ((i = 0; i < ${#fs_dirs[@]}; i++)); do
        if [ -z "${bug_dirs[i]}" ]; then
            echo "DIRNAMES ${fs_dirs[i]}"
            exit 1
            #return 1
        fi
    done

    # Check mode and size
    for ((i = 0; i < ${#file_paths[@]}; i++)); do
        path="${file_paths[i]}"
        # Check if stat is correct
        bug_stat=$(stat -c '%f %s' "$path")
        comp=${fs_stats[i]}

        # echo "$path"
        # echo "ACTUAL: $comp vs BUG: $bug_stat"
        if [ "$bug_stat" != "$comp" ]; then
            echo "MODE $path"
            #exit 1
            #return 1
        fi


    done

    #Check cksum
    index=0
    for fn in "${keys[@]}"; do
        path="${file_to_path[$fn]}"
        check="${fs_cksum[index]}"
        # do long and short read and compare with cksum
        # read 97 bytes at a time:
        bug_cksum_short=$(dd if="$path" bs=4096 status=none | cksum)

        # echo "$path"
        # echo "Actual: $check vs Bug: $bug_cksum_short"
        if [ "$bug_cksum_short" != "$check" ]; then
            echo "CKSUM $fn"
            exit 1
            #return 1
        fi
        #read 5000 bytes at a time:
        bug_cksum_long=$(dd if="$path" bs=5000 status=none | cksum)
        if [ "$bug_cksum_long" != "$check" ]; then
            echo "CKSUM $fn"
            exit 1
            #return 1
        fi
        index=$((index + 1))
    done

    

}
testReadDirs

# List of bugs to test
# bugs=("15" "1" "2" "3" "4" "5" "6" "7" "8" "9" "10" "11" "12" "13" "14")

# # Iterate over each bug and run tests
# for bug in "${bugs[@]}"; do
#     make umount
#     sleep 2
#     ./lab4-bug.aarch64 -bug "$bug" -image test.img fs
#     sleep 2
#     echo "$bug"
 
#     # Run directory read test
#     testReadDirs
# done
