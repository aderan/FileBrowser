
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "fb_common.h"
#include "json.h"

#define LOCAL_LOG_TAG "filebrowser_common"
#include "android_log.h"


char *fb_convert(FileItem **items){
    FileItem **item_move = items;
    json_t *j_items,*j_item,*label,*value;
    char numtos[32],*buf;

    j_items = json_new_array();
    while (*item_move != NULL){
        j_item = json_new_object();

        label = json_new_string("sourceType");
        value = json_new_string((*item_move)->sourcetype);
        json_insert_child(label, value);
        json_insert_child(j_item, label);

        label = json_new_string("isdir");
        value = json_new_string((*item_move)->isdir?"true":"false");
        json_insert_child(label, value);
        json_insert_child(j_item, label);


        label = json_new_string("permission");
        snprintf(numtos, sizeof numtos, "%x",((*item_move)->permission));
        value = json_new_string(numtos);
        json_insert_child(label, value);
        json_insert_child(j_item, label);

        label = json_new_string("size");
        snprintf(numtos, sizeof numtos, "%d",((*item_move)->size));
        value = json_new_string(numtos);
        json_insert_child(label, value);
        json_insert_child(j_item, label);

        label = json_new_string("name");
        value = json_new_string((*item_move)->name);
        json_insert_child(label, value);
        json_insert_child(j_item, label);

        label = json_new_string("fullpath");
        value = json_new_string((*item_move)->fullpath);
        json_insert_child(label, value);
        json_insert_child(j_item, label);

        label = json_new_string("ctime");
        snprintf(numtos, sizeof numtos, "%lu",((*item_move)->ctime));
        value = json_new_string(numtos);
        json_insert_child(label, value);
        json_insert_child(j_item, label);

        label = json_new_string("atime");
        snprintf(numtos, sizeof numtos, "%lu",((*item_move)->atime));
        value = json_new_string(numtos);
        json_insert_child(label, value);
        json_insert_child(j_item, label);

        label = json_new_string("mtime");
        snprintf(numtos, sizeof numtos, "%lu",((*item_move)->mtime));
        value = json_new_string(numtos);
        json_insert_child(label, value);
        json_insert_child(j_item, label);

        int i;
        if ((*item_move)->uris_num != 0){
            json_t *j_uris =  json_new_array();
            for (i=0;i<(*item_move)->uris_num;i++)
            {
                value = json_new_string((*item_move)->uris[i]);
                json_insert_child(j_uris,value);
            }
            label = json_new_string("urls");
            json_insert_child(label, j_uris);
            json_insert_child(j_item,label);
        }

        if ((*item_move)->icons_num != 0){
            json_t *j_icons =  json_new_array();
            for (i=0;i<(*item_move)->icons_num;i++)
            {
                json_t *j_icon_item = json_new_object();
                snprintf(numtos, sizeof numtos, "%d",(*item_move)->icons[i].width);
                value = json_new_string(numtos);
                json_insert_pair_into_object(j_icon_item,"width",value);
                snprintf(numtos, sizeof numtos, "%d",(*item_move)->icons[i].height);
                value = json_new_string(numtos);
                json_insert_pair_into_object(j_icon_item,"height",value);
                value = json_new_string((*item_move)->icons[i].uri);
                json_insert_pair_into_object(j_icon_item,"uri",value);

                json_insert_child(j_icons,j_icon_item);
            }
            label = json_new_string("icons");
            json_insert_child(label, j_icons);
            json_insert_child(j_item,label);
        }

        if ((*item_move)->thumbnails_num != 0){
            json_t *j_icons =  json_new_array();
            for (i=0;i<(*item_move)->thumbnails_num;i++)
            {
                json_t *j_thum_item = json_new_object();
                snprintf(numtos, sizeof numtos, "%d",(*item_move)->thumbnails[i].width);
                value = json_new_string(numtos);
                json_insert_pair_into_object(j_thum_item,"width",value);
                snprintf(numtos, sizeof numtos, "%d",(*item_move)->thumbnails[i].height);
                value = json_new_string(numtos);
                json_insert_pair_into_object(j_thum_item,"height",value);
                value = json_new_string((*item_move)->thumbnails[i].uri);
                json_insert_pair_into_object(j_thum_item,"uri",value);

                json_insert_child(j_icons,j_thum_item);
            }
            label = json_new_string("thumbnails");
            json_insert_child(label, j_icons);
            json_insert_child(j_item,label);
        }
        //For More
        json_insert_child(j_items, j_item);
        item_move = &(*item_move)->next;
    }
    json_tree_to_string(j_items, &buf);
    json_free_value(&j_items);

    return buf;
}
void fb_free_items(FileItem *items){
    FileItem *item_move = items;

    while (items != NULL){
        item_move = items->next;
        free (items);
        items = item_move;
    }
}

