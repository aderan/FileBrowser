#this this for unix smb
smb_test_1:
	gcc -o smb_test_1 -DCASE1 fb_utils.c fb_smb.c  readdir_test_smb.c json.c unix/libsmbclient.a -ldl -lresolv fb_file.c
