/*
 Authors:
 Copyright 2012-2015 by Eduard Broese <ed.broese@gmx.de>

 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 as published by the Free Software Foundation; either version
 2 of the License, or (at your option) any later version.
*/

#include "mcp_database.h"
#include <json-c/json.h>
#include <curl/curl.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include "lh_buffers.h"
#include "lh_files.h"
#include "lh_dir.h"

#define TESTEXAMPLES //show test examples after loading db

char *databasefilepath = "./database";

lh_arr_declare(database_t,dbs);

database_t *activedb;  //pointer to the db for the current connection's protocol

//forward declaration
int try_to_get_missing_json_files(int protocol_id);
int save_db_to_file(database_t *db);
int load_db_from_file(database_t *db, FILE* fp);
int test_examples();

// Loads a db for this protocol into memory
int db_load(int protocol_id) {
    activedb = NULL;

    //check if the db for this protocol is already loaded
    for (int i=0; i<C(dbs); i++) {
        if ( P(dbs)[i].protocol == protocol_id ) {
            printf("Database %d already loaded, setting the activedb pointer.\n",i);
            activedb = &P(dbs)[i];
            #ifdef TESTEXAMPLES
                test_examples();
            #endif
            return 0; //success
        }
    }

    //need a new database in the array of databases
    database_t *newdb = lh_arr_new_c(GAR(dbs));

    //TODO: Protocol Specific
    assert(protocol_id == 751);

    char blockjsonfilespec[PATH_MAX], itemjsonfilespec[PATH_MAX], dbfilespec[PATH_MAX];
    sprintf(blockjsonfilespec, "%s/blocks_%d.json",databasefilepath,protocol_id);  //example "./database/blocks_404.json"
    sprintf(itemjsonfilespec, "%s/items_%d.json",databasefilepath,protocol_id);  //example: "./database/items_404.json"
    sprintf(dbfilespec, "%s/mcb_db_%d.txt",databasefilepath,protocol_id);  //example: "./database/mcb_db_404.txt"

    //First Method: if the db file already exists, load it.
    FILE *dbfile = fopen(dbfilespec, "r");
    if (dbfile) {
        int rc = load_db_from_file(newdb, dbfile);
        fclose(dbfile);
        if (!rc) {
            printf("Database successfully loaded from %s\n",dbfilespec);
            activedb = newdb;
            #ifdef TESTEXAMPLES
                test_examples();
            #endif
            return 0; //success
        }
        else {
            //unload failed attempt move to method 2 or 3
            //db_unload();
        }
    }

    //Second Method: Check for the proper blocks.json and items.json in the database directory
    FILE *f1 = fopen(blockjsonfilespec, "r");
    FILE *f2 = fopen(itemjsonfilespec, "r");
    //TODO: Logic error leaves a file open if only 1 of the files exists
    if ( f1 && f2 ) {
        fclose(f1);
        fclose(f2);
        printf("Loading %s and %s\n",blockjsonfilespec,itemjsonfilespec);
    }
    else {
        printf("%s and/or %s not found or available.\n",blockjsonfilespec,itemjsonfilespec);
        //Third Method: Attempt to download server.jar and extract them from it
        int rc = try_to_get_missing_json_files(protocol_id);
        if (rc) {
            printf("Error couldnt load database\n");
            return -1; //return code - failure
        }
        //Now the files are there
    }

    newdb->protocol = protocol_id;

    //////////////////////////////////
    //  Load blocks.json into db    //
    //////////////////////////////////

    //load the blocks.json file into a parsed json object in memory
    json_object *jblkmain, *blockstatejson, *blockstatesarrayjson, *blockidjson, *blockpropjson, *blockdefaultidjson;
    jblkmain = json_object_from_file (blockjsonfilespec);

    int namecount = 0;
    //loop through each block(name) which is the top level of the json
    json_object_object_foreach(jblkmain, keymain, valmain) {

        //valmain is more json with { properties: ... , states: [ ... , ... ] } and we want the states.
        json_object_object_get_ex(valmain,"states", &blockstatesarrayjson);

        //first get the default blockid for this blockname -- needed when populating the database in the second loop below
        int defaultid = -1;
        //get the number of states (blockids) for this blockname
        int statesarrlen = json_object_array_length(blockstatesarrayjson);
        //loop through each blockid looking for which one is the default
        for (int i=0; i<statesarrlen; i++) {
            blockstatejson = json_object_array_get_idx(blockstatesarrayjson, i);
            //look for the default:true keypair
            json_object_object_get_ex(blockstatejson,"default", &blockdefaultidjson);
            if(json_object_get_boolean(blockdefaultidjson)) {
                //found the default:true keypair so get the block id of this state and save it as the default id
                json_object_object_get_ex(blockstatejson,"id", &blockdefaultidjson);
                defaultid = json_object_get_int(blockdefaultidjson);
                break;
            }
        }
        //now that we have the default id for this blockname, loop through every state again to populate the database
        //each state's json contains { id: ...,  [default]:..., [properties]: {...,...} }
        for (int i=0; i<statesarrlen; i++) {
            //allocate a new block in the db array
            block_t *blk = lh_arr_new_c(GAR(newdb->block));

            //get this block state's json
            blockstatejson = json_object_array_get_idx(blockstatesarrayjson, i);

            //get the block id within this state
            json_object_object_get_ex(blockstatejson,"id", &blockidjson);
            int blkid = json_object_get_int(blockidjson);

            //get the properties if they exist
            json_object_object_get_ex(blockstatejson,"properties", &blockpropjson);

            if (json_object_get_type(blockpropjson) == json_type_object) {
                //this block has properties -- loop through each property tp get the key : value
                json_object_object_foreach(blockpropjson, keyprop, valprop) {
                    //allocate a new property array element within this block record
                    prop_t *prp = lh_arr_new_c(GAR(blk->prop));

                    // Property Name: allocate memory , copy into memory, and store in database
                    char* propertyname = malloc(strlen(keyprop)+1);
                    strcpy(propertyname, keyprop);
                    prp->pname = propertyname;

                    // Property Value: allocate memory , copy into memory, and store in database
                    char* propertyval = malloc(strlen(json_object_get_string(valprop))+1);
                    strcpy(propertyval, json_object_get_string(valprop));
                    prp->pvalue = propertyval;
                }
            }
            // confirm that blockname begins with "minecraft:"
            assert(!strncmp(keymain,"minecraft:",10));
            // pointer arithmetic to get rid of the minecraft:
            blk->name = keymain+10;
            blk->id = blkid;
            blk->oldid = namecount;
            blk->defaultid = defaultid;
        }
    namecount++;
    }

    //////////////////////////////////
    //  Load items.json into db     //
    //////////////////////////////////

    json_object *jobjfile, *jobjitems, *jobj, *itemidstructurejson, *itemidjson;

    //load the items.json file into a parsed json object in memory
    jobjfile = json_object_from_file (itemjsonfilespec);

    //there are many registries not just items anymore (1.16.2) so find the items json
    int found=0;
    found = json_object_object_get_ex(jobjfile,"minecraft:item", &jobjitems);
    if(!found) printf("Error finding minecraft:item\n");
    found = json_object_object_get_ex(jobjitems,"entries", &jobj);
    if(!found) printf("Error finding entries\n");


     //initialize an iterator. "it" is the iterator and "itEnd" is the end of the json where the iterations stop
    struct json_object_iterator it = json_object_iter_begin(jobj);
    struct json_object_iterator itEnd = json_object_iter_end(jobj);

    //loop through each record of the json
    //note structure is { item_name : { "protocol_id" : value } } so the value is nested within another json
    while (!json_object_iter_equal(&it, &itEnd)) {

        // the name of the json record is the item_name itself
        const char* itemname = json_object_iter_peek_name(&it);

        // the value of this pair is another json so go one level deeper for the item_id
        itemidstructurejson = json_object_iter_peek_value(&it);

        // process the internal json, look for the key "protocol_id" and get its value (the item_id)
        json_object_object_get_ex(itemidstructurejson,"protocol_id", &itemidjson);

        // convert the value of protocol_id from an abstract object into an integer
        int itemid = json_object_get_int(itemidjson);

        // confirm that item name begins with "minecraft:"
        assert(!strncmp(itemname,"minecraft:",10));

        //create another element in the item array
        item_t *itm = lh_arr_new_c(GAR(newdb->item));

        //store the item name into dbrecord using pointer arithmetic to get rid of the "minecraft:"
        itm->name = itemname + 10;

        //store the item_id into dbrecord
        itm->id = itemid;

        // iterate through the json iterator
        json_object_iter_next(&it);
    }
    save_db_to_file(newdb);
    activedb = newdb;
    #ifdef TESTEXAMPLES
        test_examples();
    #endif

    return 0; //success
}

// Gets the item id given the item name
int db_get_item_id(const char *name) {
    assert(activedb);
    for (int i =0; i < C(activedb->item); i++) {
        if (!strcmp(activedb->P(item)[i].name, name)) {
            return activedb->P(item)[i].id;
        }
    }
    //TODO: if this is a blockname with a different itemname for the base material
    return -1;
};

// Gets the item name given the item id
const char *db_get_item_name(int item_id) {
    assert (activedb);
    if (item_id == -1) {
        return "Empty";
    }
    for (int i=0; i < C(activedb->item); i++) {
        if (item_id == activedb->P(item)[i].id) {
            return activedb->P(item)[i].name;
        }
    }
    return "ID not found";
};

// Private:  Gets the database record for a block given its block_id
block_t *db_blk_record_from_id(blid_t block_id) {
    for (int i=0; i < C(activedb->block); i++) {
        if (block_id == P(activedb->block)[i].id) {
            return &P(activedb->block)[i];
        }
    }
    return NULL;
}

// Gets the block name given the block id
const char *db_get_blk_name(blid_t id) {
    assert(activedb);
    block_t *blk = db_blk_record_from_id(id);
    if (blk) return blk->name;
    return "ID not found";
};

// Gets the blockname from the old-style block ID (only relevant for the SP_BlockAction packet)
const char *db_get_blk_name_from_old_id(blid_t oldid) {
    assert(activedb);
    char *buf;
    if (oldid > P(activedb->block)[C(activedb->block)-1].oldid) {
        return "ID out of bounds";
    }
    for (int i=0; i < C(activedb->block); i++) {
        if (oldid == P(activedb->block)[i].oldid) {
            return P(activedb->block)[i].name;
        }
    }
    return "ID not found";
};

// Gets the block's default id given a block name
//  db_get_blk_id("cobblestone") => 14
//  db_get_blk_id("nether_brick_stairs") => 4540 // north,bottom,straight,false marked as default
blid_t db_get_blk_id(const char *name) {
    assert (activedb);
    for (int i =0; i < C(activedb->block); i++) {
        if (!strcmp(activedb->P(block)[i].name, name)) {
            return activedb->P(block)[i].defaultid;
        }
    }
    return UINT16_MAX;
}

// input is another block id, returning that block id's default id
blid_t db_get_blk_default_id(blid_t id) {
    assert (activedb);
    block_t *blk = db_blk_record_from_id(id);
    if (blk) return blk->defaultid;
    return UINT16_MAX;
}

// Gets the number of states a block has, given the block id
//  db_get_num_states(5) => 1 // polished_diorite
//  db_get_num_states(8) => 2 // grass_block
int db_get_num_states(blid_t block_id) {
    assert (activedb);
    int count = 0;
    //we could use defaultid or blockname since they are 1-1 correspondence
    int defid = db_get_blk_default_id(block_id);
    for (int i=0; i < C(activedb->block); i++) {
        if (P(activedb->block)[i].defaultid == defid) {
            count++;
        }
    }
    return count;
}

// Dumps blocks array to stdout
void db_dump_blocks(int maxlines){
    assert (activedb);
    printf("Dumping Blocks...\n");
    printf("%-30s %-5s %-5s %-5s %s\n","blockname","blkid","oldid","defid","#prop");
    for (int i=0; (i < C(activedb->block)) && (i < maxlines); i++) {
        printf("%30s ", P(activedb->block)[i].name);
        printf("%05u ", P(activedb->block)[i].id);
        printf("%05u ", P(activedb->block)[i].oldid);
        printf("%05u ", P(activedb->block)[i].defaultid);
        printf("%05zd ", P(activedb->block)[i].C(prop));
        for (int j=0; j < P(activedb->block)[i].C(prop); j++) {
            printf("prop:%s val:%s, ", P(activedb->block)[i].P(prop)[j].pname, P(activedb->block)[i].P(prop)[j].pvalue);
        }
        printf("\n");
    }
}

// Dumps items array to stdout
void db_dump_items(int maxlines){
    assert (activedb);
    printf("Dumping Items...\n");
    for (int i=0; (i < C(activedb->item)) && (i < maxlines) ; i++) {
        printf("%30s ", activedb->P(item)[i].name);
        printf("%05d ", activedb->P(item)[i].id);
        printf("\n");
    }
}

// Dumps blocks array to a csv file
int db_dump_blocks_to_csv_file() {
    assert (activedb);
    char *blockcsvfilespec = malloc(strlen(databasefilepath)+30);
    sprintf(blockcsvfilespec, "%s/mcb_db_%d_blocks.csv",databasefilepath,activedb->protocol);
    FILE *fp = fopen(blockcsvfilespec,"w");
    if  ( fp == NULL ) {
        printf("Can't open csv file\n");
        return -1;
    }
    fprintf(fp, "%s, %s, %s, %s, %s\n","blockname","blkid","oldid","defid","#prop");
    for (int i=0; i < C(activedb->block) ; i++) {
        fprintf(fp, "%s,", P(activedb->block)[i].name);
        fprintf(fp, "%u,", P(activedb->block)[i].id);
        fprintf(fp, "%u,", P(activedb->block)[i].oldid);
        fprintf(fp, "%u,", P(activedb->block)[i].defaultid);
        fprintf(fp, "%zd", P(activedb->block)[i].C(prop));
        for (int j=0; j < P(activedb->block)[i].C(prop); j++) {
            fprintf(fp, ",prop:%s val:%s", P(activedb->block)[i].P(prop)[j].pname, P(activedb->block)[i].P(prop)[j].pvalue);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    return 0;
}

// Dumps items array to a csv file
int db_dump_items_to_csv_file() {
    assert (activedb);
    char *itemcsvfilespec = malloc(strlen(databasefilepath)+30);
    sprintf(itemcsvfilespec, "%s/mcb_db_%d_items.csv",databasefilepath,activedb->protocol);
    FILE *fp = fopen(itemcsvfilespec,"w");
    if  ( fp == NULL ) {
        printf("Can't open csv file\n");
        return -1;
    }
    fprintf(fp, "%s,%s\n", "itemname","id");
    for (int i=0; i < C(activedb->item) ; i++) {
        fprintf(fp, "%s,", activedb->P(item)[i].name);
        fprintf(fp, "%d\n", activedb->P(item)[i].id);
    }
    fclose(fp);
    return 0;
}

// Private: Saves a database struct to file so it can be loaded in the future
int save_db_to_file(database_t *db) {
    char *dbfilespec = malloc(strlen(databasefilepath)+20);
    sprintf(dbfilespec, "%s/mcb_db_%d.txt",databasefilepath,db->protocol);  //example: "./database/mcb_db_404.txt"
    FILE *fp = fopen(dbfilespec,"w");
    if  ( fp == NULL ) {
        printf("Can't open txt file\n");
        return -1;
    }
    fprintf(fp, "%d\n", db->protocol);
    fprintf(fp, "%zd\n", C(db->item));
    fprintf(fp, "%zd\n", C(db->block));
    for (int i=0; i < C(db->item); i++) {
        fprintf(fp, "%d\n",db->P(item)[i].id);
        fprintf(fp, "%s\n",db->P(item)[i].name);
    }
    for (int i=0; i < C(db->block); i++) {
        fprintf(fp, "%s\n",P(db->block)[i].name);
        fprintf(fp, "%d\n",P(db->block)[i].id);
        fprintf(fp, "%d\n",P(db->block)[i].oldid);
        fprintf(fp, "%d\n",P(db->block)[i].defaultid);
        fprintf(fp, "%zd\n",P(db->block)[i].C(prop));
        for (int j=0; j < P(db->block)[i].C(prop); j++) {
            fprintf(fp, "%s\n",P(db->block)[i].P(prop)[j].pname);
            fprintf(fp, "%s\n",P(db->block)[i].P(prop)[j].pvalue);
        }
    }
    fclose(fp);
    return 0;
}

// Private: Loads a previously-saved db file created by save_db_to_file
int load_db_from_file(database_t *db, FILE* fp) {

    char buf[100];
    int itemcount, blockcount,propcount;

    fgets(buf,100,fp);
    db->protocol = atoi(buf);
    fgets(buf,100,fp);
    itemcount = atoi(buf);
    fgets(buf,100,fp);
    blockcount = atoi(buf);
    for (int i=0; i < itemcount; i++) {
        item_t *itm = lh_arr_new_c(GAR(db->item));
        fgets(buf,100,fp);
        itm->id = atoi(buf);
        fgets(buf,100,fp);
        buf[strcspn(buf, "\n")] = 0;
        char *itemname = malloc(strlen(buf)+1);
        strcpy(itemname,buf);
        itm->name = itemname;
    }
    for (int i=0; i < blockcount; i++) {
        block_t *blk = lh_arr_new_c(GAR(db->block));
        fgets(buf,100,fp);
        buf[strcspn(buf, "\n")] = 0;
        char *blockname = malloc(strlen(buf)+1);
        strcpy(blockname,buf);
        blk->name = blockname;
        fgets(buf,100,fp);
        blk->id = atoi(buf);
        fgets(buf,100,fp);
        blk->oldid = atoi(buf);
        fgets(buf,100,fp);
        blk->defaultid = atoi(buf);
        fgets(buf,100,fp);
        propcount = atoi(buf);
        for (int j=0; j < propcount; j++) {
            prop_t *prp = lh_arr_new_c(GAR(blk->prop));
            fgets(buf,100,fp);
            buf[strcspn(buf, "\n")] = 0;
            char *propname = malloc(strlen(buf)+1);
            strcpy(propname,buf);
            prp->pname = propname;
            fgets(buf,100,fp);
            buf[strcspn(buf, "\n")] = 0;
            char *propvalue = malloc(strlen(buf)+1);
            strcpy(propvalue,buf);
            prp->pvalue = propvalue;
        }
    }
    if (fgets(buf,100,fp)) {
        printf("Error loading db from file..too many lines\n");
        return -1;
    }
    return 0;
}

// Unloads all databases from memory
void db_unload() {
    activedb = NULL;
    int dbcount = C(dbs);
    printf("Database array contains %d array(s).\n",dbcount);
    //process each database
    for (int i=0; i < dbcount; i++) {
        //give this database a convenient name
        database_t db = P(dbs)[i];
        int itemcount = db.C(item);
        int blockcount = db.C(block);
        //loop through each item freeing its name
        for (int j=0; j< itemcount; j++) {
            free((char*)db.P(item)[j].name);
        }
        //loop through each block
        for (int k=0; k < blockcount; k++) {
            assert (k < blockcount);
            //give this block a convenient name
            block_t blk = db.P(block)[k];
            //loop its properties to free propname/val
            for (int l=0; l<blk.C(prop); l++) {
                free((char*)blk.P(prop)[l].pname);
                free((char*)blk.P(prop)[l].pvalue);
            }
            //free this blocks prop array
            lh_arr_free(GAR(blk.prop));
            //free this blocks name string
            free((char*)blk.name);
        }
        //now all blocks and items are cleared, so free the arrays
        lh_arr_free(GAR(db.item));
        lh_arr_free(GAR(db.block));
    }
    lh_arr_free(GAR(dbs));
    return;
}

// Gets the property value of a property for this block id
//  db_get_blk_propval(db,14,"facing") => NULL // no such property
//  db_get_blk_propval(db,1650,"facing") => "north"
//  db_get_blk_propval(db,1686,"half") => "bottom"
const char *db_get_blk_propval(blid_t block_id, const char *propname) {
    assert (activedb);
    block_t *blk = db_blk_record_from_id(block_id);
    if (blk) {
        for (int j=0; j < blk->C(prop); j++) {
            if (!strcmp(blk->P(prop)[j].pname, propname)) {
                return blk->P(prop)[j].pvalue;
            }
        }
    }
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////

// block types we should exclude from scanning
int db_blk_is_noscan(blid_t blk_id) {
    const char *blk_name = db_get_blk_name(blk_id);
    if (!strcmp(blk_name, "air")) return 1;
    if (!strcmp(blk_name, "water")) return 1;
    if (!strcmp(blk_name, "lava")) return 1;
    if (!strcmp(blk_name, "grass")) return 1;
    if (!strcmp(blk_name, "seagrass")) return 1;
    if (!strcmp(blk_name, "tall_seagrass")) return 1;
    if (!strcmp(blk_name, "fire")) return 1;
    if (!strcmp(blk_name, "snow")) return 1;
    if (!strcmp(blk_name, "nether_portal")) return 1;
    if (!strcmp(blk_name, "end_portal")) return 1;
    if (!strcmp(blk_name, "carrots")) return 1;
    if (!strcmp(blk_name, "potatoes")) return 1;
    if (!strcmp(blk_name, "beetroots")) return 1;
    return 0;
}

// block types that are considered 'empty' for the block placement
int db_blk_is_empty(blid_t blk_id) {
    const char *blk_name = db_get_blk_name(blk_id);
    if (!strcmp(blk_name, "air")) return 1;
    if (!strcmp(blk_name, "water")) return 1;
    if (!strcmp(blk_name, "lava")) return 1;
    if (!strcmp(blk_name, "grass")) return 1;
    if (!strcmp(blk_name, "seagrass")) return 1;
    if (!strcmp(blk_name, "tall_seagrass")) return 1;
    if (!strcmp(blk_name, "fire")) return 1;
    if (!strcmp(blk_name, "snow")) return 1;
    return 0;
}

// blocks that are onwall -- cannot use item flags -- the block & item names dont match
int db_blk_is_onwall(blid_t blk_id) {
    const char *blk_name = db_get_blk_name(blk_id);
    if (!strcmp(blk_name, "wall_torch")) return 1;
    if (!strcmp(blk_name, "wall_sign")) return 1;
    if (!strcmp(blk_name, "redstone_wall_torch")) return 1;
    if (!strcmp(blk_name, "skeleton_wall_skull")) return 1;
    if (!strcmp(blk_name, "wither_skeleton_wall_skull")) return 1;
    if (!strcmp(blk_name, "zombie_wall_head")) return 1;
    if (!strcmp(blk_name, "player_wall_head")) return 1;
    if (!strcmp(blk_name, "creeper_wall_head")) return 1;
    if (!strcmp(blk_name, "dragon_wall_head")) return 1;
    if (!strcmp(blk_name, "white_wall_banner")) return 1;
    if (!strcmp(blk_name, "orange_wall_banner")) return 1;
    if (!strcmp(blk_name, "magenta_wall_banner")) return 1;
    if (!strcmp(blk_name, "light_blue_wall_banner")) return 1;
    if (!strcmp(blk_name, "yellow_wall_banner")) return 1;
    if (!strcmp(blk_name, "lime_wall_banner")) return 1;
    if (!strcmp(blk_name, "pink_wall_banner")) return 1;
    if (!strcmp(blk_name, "gray_wall_banner")) return 1;
    if (!strcmp(blk_name, "light_gray_wall_banner")) return 1;
    if (!strcmp(blk_name, "cyan_wall_banner")) return 1;
    if (!strcmp(blk_name, "purple_wall_banner")) return 1;
    if (!strcmp(blk_name, "blue_wall_banner")) return 1;
    if (!strcmp(blk_name, "brown_wall_banner")) return 1;
    if (!strcmp(blk_name, "green_wall_banner")) return 1;
    if (!strcmp(blk_name, "red_wall_banner")) return 1;
    if (!strcmp(blk_name, "black_wall_banner")) return 1;
    if (!strcmp(blk_name, "dead_tube_coral_wall_fan")) return 1;
    if (!strcmp(blk_name, "dead_brain_coral_wall_fan")) return 1;
    if (!strcmp(blk_name, "dead_bubble_coral_wall_fan")) return 1;
    if (!strcmp(blk_name, "dead_fire_coral_wall_fan")) return 1;
    if (!strcmp(blk_name, "dead_horn_coral_wall_fan")) return 1;
    if (!strcmp(blk_name, "tube_coral_wall_fan")) return 1;
    if (!strcmp(blk_name, "brain_coral_wall_fan")) return 1;
    if (!strcmp(blk_name, "bubble_coral_wall_fan")) return 1;
    if (!strcmp(blk_name, "fire_coral_wall_fan")) return 1;
    if (!strcmp(blk_name, "horn_coral_wall_fan")) return 1;
    return 0;
}

// Gets the block_id that matches another block_id, except for changing one property to a different value
blid_t db_blk_property_change(blid_t blk_id, const char* prop_name, const char* new_prop_value) {
    block_t *originalblk = db_blk_record_from_id(blk_id);
    int propertycount = originalblk->C(prop);

    // loop through every block in the database to find a match
    for (int i=0; i < activedb->C(block); i++) {
        block_t newblk = activedb->P(block)[i];
        if (!strcmp( newblk.name, originalblk->name) ) {
            //found a block with the same block name - loop all properties for a match
            int matchcount = 0;
            for (int j=0; j < propertycount; j++) {
                // if this is the property that should change and its value matches new_prop_value
                if (!strcmp(newblk.P(prop)[j].pname, prop_name)) {
                    if (!strcmp(newblk.P(prop)[j].pvalue, new_prop_value)) matchcount++;
                }
                // this is one of the properties that should remain the same as the original block
                else if (!strcmp(newblk.P(prop)[j].pvalue, originalblk->P(prop)[j].pvalue)) matchcount++;
            }
            if ( matchcount == propertycount ) return newblk.id;
        }
    }
    return UINT16_MAX; //not found
}

// takes a block_id and returns the id that matches it, except for the facing axis property rotated in degrees
blid_t db_get_rotated_block(blid_t blk_id, int degrees) {
    assert (activedb);
    assert (degrees == 90 || degrees == 180 || degrees == 270);

    const char *currentdirection = db_get_blk_propval(blk_id,"facing");
    const char *currentaxis = db_get_blk_propval(blk_id,"axis");
    block_t *blk = db_blk_record_from_id(blk_id);

    if (currentdirection) {
        char *desireddirection;
        //TODO: something more elegant
        if (!strcmp(currentdirection, "north")) {
            if (degrees == 90) desireddirection = "east";
            if (degrees == 180) desireddirection = "south";
            if (degrees == 270) desireddirection = "west";
        }
        else if (!strcmp(currentdirection, "east")) {
            if (degrees == 90) desireddirection = "south";
            if (degrees == 180) desireddirection = "west";
            if (degrees == 270) desireddirection = "north";
        }
        else if (!strcmp(currentdirection, "south")) {
            if (degrees == 90) desireddirection = "west";
            if (degrees == 180) desireddirection = "north";
            if (degrees == 270) desireddirection = "east";
        }
        else if (!strcmp(currentdirection, "west")) {
            if (degrees == 90) desireddirection = "north";
            if (degrees == 180) desireddirection = "east";
            if (degrees == 270) desireddirection = "south";
        }
        else return blk_id;  //block is facing up or down

        return db_blk_property_change(blk_id, "facing", desireddirection);
    }
    else if (currentaxis) {
        char *desiredaxis;
        if (!strcmp(currentaxis, "x")) {
            if (degrees == 90) desiredaxis = "z";
            if (degrees == 180) return blk_id;
            if (degrees == 270) desiredaxis = "z";
        }
        else if (!strcmp(currentaxis, "z")) {
            if (degrees == 90) desiredaxis = "x";
            if (degrees == 180) return blk_id;
            if (degrees == 270) desiredaxis = "x";
        }
        else return blk_id; //block in y axis

        return db_blk_property_change(blk_id, "axis", desiredaxis);
    }
    return blk_id;  //block doesnt rotate or have an axis (or slipped thru horrible logic above)
}

// Private: Get all block records matching a given blockname
int get_all_records_matching_blockname(const char *blockname, block_t *blockarray[] ) {
    assert(activedb);
    int count = 0;
    for (int i=0; i < activedb->C(block); i++) {
        if (!strcmp(activedb->P(block)[i].name, blockname)) {
            blockarray[count]= &activedb->P(block)[i];
            count++;
        }
    }
    return count;
}

// Private: Get a property value given a block record and property name
const char *get_prop_value_from_record(block_t *blk, const char *propname) {
    for (int j=0; j < blk->C(prop); j++) {
        if (!strcmp(blk->P(prop)[j].pname, propname)) {
            return blk->P(prop)[j].pvalue;
        }
    }
    return NULL;
}

// places all ids matching a set of propeties for the block name into array ids (can be assumed to be long enough) and returns the number of ids
int db_get_matching_block_ids(const char *name, prop_t *match, int propcount, blid_t *ids) {
    block_t *blockarray[2000];
    int blkcount = get_all_records_matching_blockname(name, blockarray);
    int blkmatches = 0;
    for (int i=0; i < blkcount; i++) {
        block_t *blk = blockarray[i]; //give this block a convenient name
        int propmatches=0;
        // loop through properties to see if this block's prop values match those in the given array
        for (int j=0; j < propcount; j++) {
            if (!strcmp(get_prop_value_from_record(blk, match[j].pname),match[j].pvalue)) propmatches++;
        }
        if (propmatches == propcount) ids[blkmatches++] = blk->id;
    }
    return blkmatches;
}

// Get all block ids matching a given blockname and places ids into an array returning the count
int db_get_all_ids_matching_blockname(const char *blockname, blid_t *idarray ) {
    assert(activedb);
    int count = 0;
    for (int i=0; i < activedb->C(block); i++) {
        if (!strcmp(activedb->P(block)[i].name, blockname)) {
            idarray[count]=activedb->P(block)[i].id;
            count++;
        }
    }
    return count;
}

////////////////////////////////////////////////////////////////////////////////

// Private:  Converts the protocol ID to its MC version string
char *protocol_to_ver_string(int protid){
    // TODO: use what we already have
    if (protid == 751) {
        return "1.16.2";
    }
    return "unknown";
}

// Private:  Saves a URL to a local file
int download_url_into_file(const char *url, const char *filespec) {
    CURL *curl = curl_easy_init();
    // set header options
    FILE *pagefile = fopen(filespec, "wb");
    if (!pagefile) return -1;
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);  //set to 1 to turn off progress
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, pagefile);
    int rc = curl_easy_perform(curl);
    fclose(pagefile);
    curl_easy_cleanup(curl);
    curl_global_cleanup();
    return rc;
}

// Private:  Finds the directory of the Java VM executable
char *findjavapath() {
    char *javapath = NULL;

#ifdef __CYGWIN__
    //all windows minecraft installs come with java.exe
    //hope their program files is default directory for now
    //but need a way to programmatically get the path of Minecraft install & the jre version it ships with
    DIR *dp;
    //Default location for 64-bit Windows OS with Minecraft 16.2
    javapath = "/cygdrive/c/Progra~2/Minecraft/runtime/jre-x64/bin";
    dp = opendir(javapath);
    if (!dp) {
        printf("findjavapath(): Directory not found\n");
        return NULL;
    }
    closedir(dp);
#endif

#ifdef __linux__
    //linux often has JAVA_HOME ready to go
    char *javahome = getenv("JAVA_HOME");
    if (!javahome) {
        return NULL;
    }
    javapath = malloc(strlen(javahome)+5);
    sprintf(javapath,"%s/bin",javahome);
#endif

    return javapath;
}

// Private:  Gets the server jar from Mojang and extract blocks.json & items.json
int try_to_get_missing_json_files(int protocol_id) {
    int rc;

    // Download version manifest from Mojang
    char *MANIFESTURL = "https://launchermeta.mojang.com/mc/game/version_manifest.json";
    char *manifestfilespec = malloc(strlen(databasefilepath)+30);
    sprintf(manifestfilespec, "%s/version_manifest.json",databasefilepath);
    rc = download_url_into_file(MANIFESTURL,manifestfilespec);
    if (rc) {
        printf ("Error downloading version manifest. %d\n",rc);
        return -1;
    }

    char *verid = protocol_to_ver_string(protocol_id);
    json_object *jman,*jver,*j132,*jid,*jurl;
    const char *urlversioninfo;

    // Parse version manifest to get the versioninfo download url for this version
    jman = json_object_from_file (manifestfilespec);
    json_object_object_get_ex(jman, "versions", &jver);
    int numversions = json_object_array_length(jver);
    for (int i=0; i<numversions; i++) {
        jid = json_object_array_get_idx(jver, i);
        json_object_object_get_ex(jid,"id", &j132);
        if (!strcmp(verid,json_object_get_string(j132))) {
            json_object_object_get_ex(jid,"url", &jurl);
            urlversioninfo = json_object_get_string(jurl);
            break;
        };
    };
    if (urlversioninfo == NULL) {
        printf ("Error parsing version manifest.\n");
        return -1;
    }

    // Download versioninfo json from Mojang
    char *verinfofilespec = malloc(strlen(databasefilepath)+30);
    sprintf(verinfofilespec, "%s/versioninfo.json",databasefilepath);
    rc = download_url_into_file(urlversioninfo,verinfofilespec);
    if (rc) {
        printf ("Error downloading versioninfo. %d\n",rc);
        return -1;
    }

    json_object *jvinfo,*jdl,*jserv,*jservurl;
    const char *urlserverjar;

    // Parse versioninfo json to get the server.jar download url for this version
    jvinfo = json_object_from_file(verinfofilespec);
    json_object_object_get_ex(jvinfo, "downloads", &jdl);
    json_object_object_get_ex(jdl, "server", &jserv);
    json_object_object_get_ex(jserv, "url", &jservurl);
    urlserverjar = json_object_get_string(jservurl);
    if (urlserverjar == NULL) {
        printf ("Error parsing versioninfo.\n");
        return -1;
    }

    char serverfilespec[50];
    snprintf(serverfilespec, sizeof(serverfilespec), "%s/server_%s.jar", databasefilepath, verid);

    // Download server.jar from Mojang
    rc = download_url_into_file(urlserverjar,serverfilespec);
    if (rc) {
        printf ("Error downloading server.jar. rc=%d\n",rc);
        return -1;
    }

    // Find path to java
    char *javafilepath = findjavapath();
    if (!javafilepath) {
        printf ("Error finding Java path.\n");
        return -1;
    }

    // run the server.jar to extract items.json and blocks.json
    char *cmd;
    cmd = malloc(strlen(javafilepath)+100);
#ifdef __CYGWIN__
    sprintf(cmd,"cd database; %s/java.exe -cp server_%s.jar net.minecraft.data.Main --all",javafilepath,verid);
#endif
#ifdef __linux__
    sprintf(cmd,"cd database; %s/java -cp server_%s.jar net.minecraft.data.Main --all",javafilepath,verid);
#endif
    FILE *fp = popen(cmd,"r");
    if (fp == NULL) {
        printf("Error launching java\n");
        return -1;
    }
    char buf[200];
    while (fgets(buf, 200, fp) != NULL) {
        printf("%s", buf);   //watch output of the java command
    }
    rc = pclose(fp);
    if (rc) {
        printf("Error processing java command %d\n",rc);
        return -1;
    }

    //Move the generated items.json and blocks.json into ./database
    char *blockjsonfilespec = malloc(strlen(databasefilepath)+20);
    sprintf(blockjsonfilespec, "%s/blocks_%d.json",databasefilepath,protocol_id);  //example "./database/blocks_404.json"

    char *itemjsonfilespec = malloc(strlen(databasefilepath)+20);
    sprintf(itemjsonfilespec, "%s/items_%d.json",databasefilepath,protocol_id);  //example: "./database/items_404.json"

    rc = rename("./database/generated/reports/registries.json", itemjsonfilespec);
    rc |= rename("./database/generated/reports/blocks.json", blockjsonfilespec);
    if (rc) {
        printf("Error couldnt move the registries.json or blocks.json to the database directory. %d\n",rc);
        return -1;
    }

    //Cleanup
    rc = system("rm -rf ./database/generated");
    rc |= system("rm -rf ./database/logs");
    rc |= remove(verinfofilespec);
    rc |= remove(manifestfilespec);
    // rc |= remove(serverfilespec);     //if we want to remove the server.jar too
    if (rc) {
        printf("Warning: Couldnt cleanup. %d",rc);
    }
    return 0;
}

// Private:  Dumps database tests to stdout (activated by #define TESTEXAMPLES)
int test_examples() {
    printf("Number of blocks: %zd\n",C(activedb->block));
    printf("Number of items: %zd\n", C(activedb->item));
    printf(" db_get_blk_propval(1686,\"half\")       = %s (bottom)\n",db_get_blk_propval(1686,"half"));
    printf(" db_get_item_id(\"heart_of_the_sea\")    = %d (789)\n", db_get_item_id("heart_of_the_sea"));
    printf(" db_get_item_name(788)                 = %s (nautilus_shell)\n", db_get_item_name(788));
    printf(" db_get_blk_name(8596)                 = %s (structure_block)\n", db_get_blk_name(8596));
    printf(" db_get_blk_default_id(8596)           = %d (8595)\n",db_get_blk_default_id(8596));
    printf(" db_get_blk_name_from_old_id(597)      = %s (structure_block)\n", db_get_blk_name_from_old_id(597));
    printf(" db_get_blk_id(\"nether_brick_stairs\")  = %d (4540)\n",db_get_blk_id("nether_brick_stairs"));
    printf(" db_get_num_states(5)                  = %d (1)\n",db_get_num_states(5));
    printf(" db_get_num_states(8)                  = %d (2)\n",db_get_num_states(8));
    printf(" db_get_num_states(2913)               = %d (1296)\n",db_get_num_states(2913));
    printf(" db_stacksize(db_get_item_id(\"ender_pearl\")) = %d (16)\n",db_stacksize(db_get_item_id("ender_pearl")));
    printf(" db_item_is_itemonly(703)              = %d (False) //skeleton_skull\n",db_item_is_itemonly(703));
    printf(" db_item_is_container(262)             = %d (True) //dropper\n",db_item_is_container(262));
    printf(" db_item_is_axis(32)                   = %d (True) //oak_log\n",db_item_is_axis(32));
    printf(" db_item_is_door(460)                  = %d (True) //iron_door\n",db_item_is_door(460));
    printf(" db_item_is_tdoor(280)                 = %d (True) //iron_trapdoor\n",db_item_is_tdoor(280));
    printf(" db_item_is_face(158)                  = %d (True) //lever\n",db_item_is_face(158));
    printf(" db_item_is_bed(596)                   = %d (True) //white_bed\n",db_item_is_bed(596));
    printf(" db_item_is_rsdev(67)                  = %d (True) //dispenser\n",db_item_is_rsdev(67));
    printf(" db_blk_is_onwall(3270)                = %d (True) //wall_sign\n",db_blk_is_onwall(3270));
    printf(" db_item_is_chest(250)                 = %d (True) //trapped_chest\n",db_item_is_chest(250));
    printf(" db_item_is_furnace(231)               = %d (True) //ender_chest\n",db_item_is_furnace(231));
    printf(" db_get_rotated_block(1649, 270)       = %d (1689) //oak_stairs rotated\n", db_get_rotated_block(1649, 270));
    printf(" db_get_rotated_block(74, 90)          = %d (72) //oak_log rotated from z to x\n", db_get_rotated_block(74, 90));
    printf(" db_blk_property_change(5315,\"facing\",\"east\") = %d (5319) //oak_button south->east\n", db_blk_property_change(5315,"facing","east"));
    printf("Now testing errors \n");
    printf(" db_get_blk_name(8599)                 = %s (out of bounds)\n", db_get_blk_name(8599));
    printf(" db_get_blk_name(8600)                 = %s (out of bounds)\n", db_get_blk_name(8600));
    printf(" db_get_blk_id(\"gold_nugget\")          = %d (UINT16_MAX meaning not found)\n",db_get_blk_id("gold_nugget"));
    printf(" db_get_num_states(8599)               = %d (0 meaning problem)\n",db_get_num_states(8599));
    printf("Now testing advanced functions\n");
    blid_t idarray[2000];
    int matchnum = db_get_all_ids_matching_blockname("lever", idarray);
    printf(" db_get_all_ids_matching_blockname(\"lever\",idarray)\n");
    for (int i=0; i<matchnum; i++) printf("%u ",idarray[i]);
    printf("\n");
    blid_t idarray2[2000];
    prop_t testproparr[2] = { {"face","wall"}, {"powered","true"} };
    int matchnum2 = db_get_matching_block_ids("lever", testproparr, 2, idarray2);
    printf(" db_get_matching_block_ids(\"lever\",{ {\"face\",\"wall\"}, {\"powered\",\"true\"} },2,idarray2)\n");
    for (int i=0; i<matchnum2; i++) printf("%u ",idarray2[i]);
    printf("\n");
    return 0;
}

///////////////////////////////////////////////////
// Item flags

// the ID is an item, i.e. cannot be placed as a block
#define I_ITEM   (1ULL<<4)

// Item does not stack (or stack size=1)
#define I_NSTACK (1ULL<<5)

// item stacks only by 16 (e.g. enderpearls)
#define I_S16    (1ULL<<6)

// Container blocks (chests, furnaces, hoppers, etc. - dialog opens if right-clicked)
#define I_CONT   (1ULL<<12)

// slab-type block - I_MPOS lower/upper placement in the meta bit 3
#define I_SLAB   (1ULL<<16)

// stair-type block - I_MPOS straight/upside-down in the meta bit 2, direction in bits 0-1
#define I_STAIR (1ULL<<17)

// blocks with an axis property - wood log type blocks
#define I_AXIS (1ULL<<18)

// redstone devices (hoppers, dispensers, droppers, pistons, observers)
#define I_RSDEV (1ULL<<23)

// doors
#define I_DOOR (1ULL<<24)

// trapdoors
#define I_TDOOR (1ULL<<25)

// chests and trapped chests - oriented and doubleable containers (type: left/right/single)
#define I_CHEST (1ULL<<27)

// buttons and lever
#define I_FACE (1ULL<<33)

// beds
#define I_BED (1ULL<<34)

// furnace and enderchest - oriented containers
#define I_FURNACE (1ULL<<35)

// item has 4 directions it can face
#define I_FACINGNESW (1ULL<<36)

// item has 5 directions it can face
#define I_FACINGNESWD (1ULL<<37)

// item has 6 directions it can face
#define I_FACINGNESWUD (1ULL<<38)

// item is adjustable (such as a repeater delay)
#define I_ADJ (1ULL<<39)

// example - placeholder should each armor type get its own designation
#define I_ARMOR 0ULL

const uint64_t item_flags[] = {
    [0] = 0,                                      //air
    [1] = 0,                                      //stone
    [2] = 0,                                      //granite
    [3] = 0,                                      //polished_granite
    [4] = 0,                                      //diorite
    [5] = 0,                                      //polished_diorite
    [6] = 0,                                      //andesite
    [7] = 0,                                      //polished_andesite
    [8] = 0,                                      //grass_block
    [9] = 0,                                      //dirt
    [10] = 0,                                     //coarse_dirt
    [11] = 0,                                     //podzol
    [12] = 0,                                     //crimson_nylium
    [13] = 0,                                     //warped_nylium
    [14] = 0,                                     //cobblestone
    [15] = 0,                                     //oak_planks
    [16] = 0,                                     //spruce_planks
    [17] = 0,                                     //birch_planks
    [18] = 0,                                     //jungle_planks
    [19] = 0,                                     //acacia_planks
    [20] = 0,                                     //dark_oak_planks
    [21] = 0,                                     //crimson_planks
    [22] = 0,                                     //warped_planks
    [23] = 0,                                     //oak_sapling
    [24] = 0,                                     //spruce_sapling
    [25] = 0,                                     //birch_sapling
    [26] = 0,                                     //jungle_sapling
    [27] = 0,                                     //acacia_sapling
    [28] = 0,                                     //dark_oak_sapling
    [29] = 0,                                     //bedrock
    [30] = 0,                                     //sand
    [31] = 0,                                     //red_sand
    [32] = 0,                                     //gravel
    [33] = 0,                                     //gold_ore
    [34] = 0,                                     //iron_ore
    [35] = 0,                                     //coal_ore
    [36] = 0,                                     //nether_gold_ore
    [37] = I_AXIS,                                //oak_log
    [38] = I_AXIS,                                //spruce_log
    [39] = I_AXIS,                                //birch_log
    [40] = I_AXIS,                                //jungle_log
    [41] = I_AXIS,                                //acacia_log
    [42] = I_AXIS,                                //dark_oak_log
    [43] = I_AXIS,                                //crimson_stem
    [44] = I_AXIS,                                //warped_stem
    [45] = I_AXIS,                                //stripped_oak_log
    [46] = I_AXIS,                                //stripped_spruce_log
    [47] = I_AXIS,                                //stripped_birch_log
    [48] = I_AXIS,                                //stripped_jungle_log
    [49] = I_AXIS,                                //stripped_acacia_log
    [50] = I_AXIS,                                //stripped_dark_oak_log
    [51] = I_AXIS,                                //stripped_crimson_stem
    [52] = I_AXIS,                                //stripped_warped_stem
    [53] = I_AXIS,                                //stripped_oak_wood
    [54] = I_AXIS,                                //stripped_spruce_wood
    [55] = I_AXIS,                                //stripped_birch_wood
    [56] = I_AXIS,                                //stripped_jungle_wood
    [57] = I_AXIS,                                //stripped_acacia_wood
    [58] = I_AXIS,                                //stripped_dark_oak_wood
    [59] = I_AXIS,                                //stripped_crimson_hyphae
    [60] = I_AXIS,                                //stripped_warped_hyphae
    [61] = I_AXIS,                                //oak_wood
    [62] = I_AXIS,                                //spruce_wood
    [63] = I_AXIS,                                //birch_wood
    [64] = I_AXIS,                                //jungle_wood
    [65] = I_AXIS,                                //acacia_wood
    [66] = I_AXIS,                                //dark_oak_wood
    [67] = I_AXIS,                                //crimson_hyphae
    [68] = I_AXIS,                                //warped_hyphae
    [69] = 0,                                     //oak_leaves
    [70] = 0,                                     //spruce_leaves
    [71] = 0,                                     //birch_leaves
    [72] = 0,                                     //jungle_leaves
    [73] = 0,                                     //acacia_leaves
    [74] = 0,                                     //dark_oak_leaves
    [75] = 0,                                     //sponge
    [76] = 0,                                     //wet_sponge
    [77] = 0,                                     //glass
    [78] = 0,                                     //lapis_ore
    [79] = 0,                                     //lapis_block
    [80] = I_CONT | I_RSDEV | I_FACINGNESWUD,     //dispenser
    [81] = 0,                                     //sandstone
    [82] = 0,                                     //chiseled_sandstone
    [83] = 0,                                     //cut_sandstone
    [84] = I_ADJ,                                 //note_block
    [85] = 0,                                     //powered_rail
    [86] = 0,                                     //detector_rail
    [87] = I_RSDEV | I_FACINGNESWUD,              //sticky_piston
    [88] = 0,                                     //cobweb
    [89] = 0,                                     //grass
    [90] = 0,                                     //fern
    [91] = 0,                                     //dead_bush
    [92] = 0,                                     //seagrass
    [93] = 0,                                     //sea_pickle
    [94] = I_RSDEV | I_FACINGNESWUD,              //piston
    [95] = 0,                                     //white_wool
    [96] = 0,                                     //orange_wool
    [97] = 0,                                     //magenta_wool
    [98] = 0,                                     //light_blue_wool
    [99] = 0,                                     //yellow_wool
    [100] = 0,                                    //lime_wool
    [101] = 0,                                    //pink_wool
    [102] = 0,                                    //gray_wool
    [103] = 0,                                    //light_gray_wool
    [104] = 0,                                    //cyan_wool
    [105] = 0,                                    //purple_wool
    [106] = 0,                                    //blue_wool
    [107] = 0,                                    //brown_wool
    [108] = 0,                                    //green_wool
    [109] = 0,                                    //red_wool
    [110] = 0,                                    //black_wool
    [111] = 0,                                    //dandelion
    [112] = 0,                                    //poppy
    [113] = 0,                                    //blue_orchid
    [114] = 0,                                    //allium
    [115] = 0,                                    //azure_bluet
    [116] = 0,                                    //red_tulip
    [117] = 0,                                    //orange_tulip
    [118] = 0,                                    //white_tulip
    [119] = 0,                                    //pink_tulip
    [120] = 0,                                    //oxeye_daisy
    [121] = 0,                                    //cornflower
    [122] = 0,                                    //lily_of_the_valley
    [123] = 0,                                    //wither_rose
    [124] = 0,                                    //brown_mushroom
    [125] = 0,                                    //red_mushroom
    [126] = 0,                                    //crimson_fungus
    [127] = 0,                                    //warped_fungus
    [128] = 0,                                    //crimson_roots
    [129] = 0,                                    //warped_roots
    [130] = 0,                                    //nether_sprouts
    [131] = 0,                                    //weeping_vines
    [132] = 0,                                    //twisting_vines
    [133] = 0,                                    //sugar_cane
    [134] = 0,                                    //kelp
    [135] = 0,                                    //bamboo
    [136] = 0,                                    //gold_block
    [137] = 0,                                    //iron_block
    [138] = I_SLAB,                               //oak_slab
    [139] = I_SLAB,                               //spruce_slab
    [140] = I_SLAB,                               //birch_slab
    [141] = I_SLAB,                               //jungle_slab
    [142] = I_SLAB,                               //acacia_slab
    [143] = I_SLAB,                               //dark_oak_slab
    [144] = I_SLAB,                               //crimson_slab
    [145] = I_SLAB,                               //warped_slab
    [146] = I_SLAB,                               //stone_slab
    [147] = I_SLAB,                               //smooth_stone_slab
    [148] = I_SLAB,                               //sandstone_slab
    [149] = I_SLAB,                               //cut_sandstone_slab
    [150] = I_SLAB,                               //petrified_oak_slab
    [151] = I_SLAB,                               //cobblestone_slab
    [152] = I_SLAB,                               //brick_slab
    [153] = I_SLAB,                               //stone_brick_slab
    [154] = I_SLAB,                               //nether_brick_slab
    [155] = I_SLAB,                               //quartz_slab
    [156] = I_SLAB,                               //red_sandstone_slab
    [157] = I_SLAB,                               //cut_red_sandstone_slab
    [158] = I_SLAB,                               //purpur_slab
    [159] = I_SLAB,                               //prismarine_slab
    [160] = I_SLAB,                               //prismarine_brick_slab
    [161] = I_SLAB,                               //dark_prismarine_slab
    [162] = 0,                                    //smooth_quartz
    [163] = 0,                                    //smooth_red_sandstone
    [164] = 0,                                    //smooth_sandstone
    [165] = 0,                                    //smooth_stone
    [166] = 0,                                    //bricks
    [167] = 0,                                    //tnt
    [168] = 0,                                    //bookshelf
    [169] = 0,                                    //mossy_cobblestone
    [170] = 0,                                    //obsidian
    [171] = 0,                                    //torch
    [172] = I_FACINGNESWUD,                       //end_rod
    [173] = 0,                                    //chorus_plant
    [174] = 0,                                    //chorus_flower
    [175] = 0,                                    //purpur_block
    [176] = I_AXIS,                               //purpur_pillar
    [177] = I_STAIR | I_FACINGNESW,               //purpur_stairs
    [178] = 0,                                    //spawner
    [179] = I_STAIR | I_FACINGNESW,               //oak_stairs
    [180] = I_CONT | I_CHEST | I_FACINGNESW,      //chest
    [181] = 0,                                    //diamond_ore
    [182] = 0,                                    //diamond_block
    [183] = I_CONT,                               //crafting_table
    [184] = 0,                                    //farmland
    [185] = I_CONT | I_FURNACE | I_FACINGNESW,    //furnace
    [186] = I_FACINGNESW,                         //ladder
    [187] = 0,                                    //rail
    [188] = I_STAIR | I_FACINGNESW,               //cobblestone_stairs
    [189] = I_FACE | I_FACINGNESW | I_ADJ,        //lever
    [190] = 0,                                    //stone_pressure_plate
    [191] = 0,                                    //oak_pressure_plate
    [192] = 0,                                    //spruce_pressure_plate
    [193] = 0,                                    //birch_pressure_plate
    [194] = 0,                                    //jungle_pressure_plate
    [195] = 0,                                    //acacia_pressure_plate
    [196] = 0,                                    //dark_oak_pressure_plate
    [197] = 0,                                    //crimson_pressure_plate
    [198] = 0,                                    //warped_pressure_plate
    [199] = 0,                                    //polished_blackstone_pressure_plate
    [200] = 0,                                    //redstone_ore
    [201] = 0,                                    //redstone_torch
    [202] = 0,                                    //snow
    [203] = 0,                                    //ice
    [204] = 0,                                    //snow_block
    [205] = 0,                                    //cactus
    [206] = 0,                                    //clay
    [207] = I_ADJ,                                //jukebox
    [208] = 0,                                    //oak_fence
    [209] = 0,                                    //spruce_fence
    [210] = 0,                                    //birch_fence
    [211] = 0,                                    //jungle_fence
    [212] = 0,                                    //acacia_fence
    [213] = 0,                                    //dark_oak_fence
    [214] = 0,                                    //crimson_fence
    [215] = 0,                                    //warped_fence
    [216] = 0,                                    //pumpkin
    [217] = I_FACINGNESW,                         //carved_pumpkin
    [218] = 0,                                    //netherrack
    [219] = 0,                                    //soul_sand
    [220] = 0,                                    //soul_soil
    [221] = 0,                                    //basalt
    [222] = 0,                                    //polished_basalt
    [223] = 0,                                    //soul_torch
    [224] = 0,                                    //glowstone
    [225] = I_FACINGNESW,                         //jack_o_lantern
    [226] = I_TDOOR | I_FACINGNESW,               //oak_trapdoor
    [227] = I_TDOOR | I_FACINGNESW,               //spruce_trapdoor
    [228] = I_TDOOR | I_FACINGNESW,               //birch_trapdoor
    [229] = I_TDOOR | I_FACINGNESW,               //jungle_trapdoor
    [230] = I_TDOOR | I_FACINGNESW,               //acacia_trapdoor
    [231] = I_TDOOR | I_FACINGNESW,               //dark_oak_trapdoor
    [232] = I_TDOOR | I_FACINGNESW,               //crimson_trapdoor
    [233] = I_TDOOR | I_FACINGNESW,               //warped_trapdoor
    [234] = 0,                                    //infested_stone
    [235] = 0,                                    //infested_cobblestone
    [236] = 0,                                    //infested_stone_bricks
    [237] = 0,                                    //infested_mossy_stone_bricks
    [238] = 0,                                    //infested_cracked_stone_bricks
    [239] = 0,                                    //infested_chiseled_stone_bricks
    [240] = 0,                                    //stone_bricks
    [241] = 0,                                    //mossy_stone_bricks
    [242] = 0,                                    //cracked_stone_bricks
    [243] = 0,                                    //chiseled_stone_bricks
    [244] = 0,                                    //brown_mushroom_block
    [245] = 0,                                    //red_mushroom_block
    [246] = 0,                                    //mushroom_stem
    [247] = 0,                                    //iron_bars
    [248] = 0,                                    //chain
    [249] = 0,                                    //glass_pane
    [250] = 0,                                    //melon
    [251] = 0,                                    //vine
    [252] = I_FACINGNESW | I_ADJ,                 //oak_fence_gate
    [253] = I_FACINGNESW | I_ADJ,                 //spruce_fence_gate
    [254] = I_FACINGNESW | I_ADJ,                 //birch_fence_gate
    [255] = I_FACINGNESW | I_ADJ,                 //jungle_fence_gate
    [256] = I_FACINGNESW | I_ADJ,                 //acacia_fence_gate
    [257] = I_FACINGNESW | I_ADJ,                 //dark_oak_fence_gate
    [258] = I_FACINGNESW | I_ADJ,                 //crimson_fence_gate
    [259] = I_FACINGNESW | I_ADJ,                 //warped_fence_gate
    [260] = I_STAIR | I_FACINGNESW,               //brick_stairs
    [261] = I_STAIR | I_FACINGNESW,               //stone_brick_stairs
    [262] = 0,                                    //mycelium
    [263] = 0,                                    //lily_pad
    [264] = 0,                                    //nether_bricks
    [265] = 0,                                    //cracked_nether_bricks
    [266] = 0,                                    //chiseled_nether_bricks
    [267] = 0,                                    //nether_brick_fence
    [268] = I_STAIR | I_FACINGNESW,               //nether_brick_stairs
    [269] = I_CONT,                               //enchanting_table
    [270] = I_FACINGNESW,                         //end_portal_frame
    [271] = 0,                                    //end_stone
    [272] = 0,                                    //end_stone_bricks
    [273] = 0,                                    //dragon_egg
    [274] = 0,                                    //redstone_lamp
    [275] = I_STAIR | I_FACINGNESW,               //sandstone_stairs
    [276] = 0,                                    //emerald_ore
    [277] = I_CONT | I_FURNACE | I_FACINGNESW,    //ender_chest
    [278] = I_FACINGNESW,                         //tripwire_hook
    [279] = 0,                                    //emerald_block
    [280] = I_STAIR | I_FACINGNESW,               //spruce_stairs
    [281] = I_STAIR | I_FACINGNESW,               //birch_stairs
    [282] = I_STAIR | I_FACINGNESW,               //jungle_stairs
    [283] = I_STAIR | I_FACINGNESW,               //crimson_stairs
    [284] = I_STAIR | I_FACINGNESW,               //warped_stairs
    [285] = I_CONT | I_FACINGNESWUD,              //command_block
    [286] = I_CONT,                               //beacon
    [287] = 0,                                    //cobblestone_wall
    [288] = 0,                                    //mossy_cobblestone_wall
    [289] = 0,                                    //brick_wall
    [290] = 0,                                    //prismarine_wall
    [291] = 0,                                    //red_sandstone_wall
    [292] = 0,                                    //mossy_stone_brick_wall
    [293] = 0,                                    //granite_wall
    [294] = 0,                                    //stone_brick_wall
    [295] = 0,                                    //nether_brick_wall
    [296] = 0,                                    //andesite_wall
    [297] = 0,                                    //red_nether_brick_wall
    [298] = 0,                                    //sandstone_wall
    [299] = 0,                                    //end_stone_brick_wall
    [300] = 0,                                    //diorite_wall
    [301] = 0,                                    //blackstone_wall
    [302] = 0,                                    //polished_blackstone_wall
    [303] = 0,                                    //polished_blackstone_brick_wall
    [304] = I_FACE | I_FACINGNESW,                //stone_button
    [305] = I_FACE | I_FACINGNESW,                //oak_button
    [306] = I_FACE | I_FACINGNESW,                //spruce_button
    [307] = I_FACE | I_FACINGNESW,                //birch_button
    [308] = I_FACE | I_FACINGNESW,                //jungle_button
    [309] = I_FACE | I_FACINGNESW,                //acacia_button
    [310] = I_FACE | I_FACINGNESW,                //dark_oak_button
    [311] = I_FACE | I_FACINGNESW,                //crimson_button
    [312] = I_FACE | I_FACINGNESW,                //warped_button
    [313] = I_FACE | I_FACINGNESW,                //polished_blackstone_button
    [314] = I_CONT | I_FACINGNESW,                //anvil
    [315] = I_CONT | I_FACINGNESW,                //chipped_anvil
    [316] = I_CONT | I_FACINGNESW,                //damaged_anvil
    [317] = I_CONT | I_CHEST | I_FACINGNESW,      //trapped_chest
    [318] = 0,                                    //light_weighted_pressure_plate
    [319] = 0,                                    //heavy_weighted_pressure_plate
    [320] = I_ADJ,                                //daylight_detector
    [321] = 0,                                    //redstone_block
    [322] = 0,                                    //nether_quartz_ore
    [323] = I_CONT | I_RSDEV | I_FACINGNESWD,     //hopper
    [324] = 0,                                    //chiseled_quartz_block
    [325] = 0,                                    //quartz_block
    [326] = 0,                                    //quartz_bricks
    [327] = I_AXIS,                               //quartz_pillar
    [328] = I_STAIR | I_FACINGNESW,               //quartz_stairs
    [329] = 0,                                    //activator_rail
    [330] = I_CONT | I_RSDEV | I_FACINGNESWUD,    //dropper
    [331] = 0,                                    //white_terracotta
    [332] = 0,                                    //orange_terracotta
    [333] = 0,                                    //magenta_terracotta
    [334] = 0,                                    //light_blue_terracotta
    [335] = 0,                                    //yellow_terracotta
    [336] = 0,                                    //lime_terracotta
    [337] = 0,                                    //pink_terracotta
    [338] = 0,                                    //gray_terracotta
    [339] = 0,                                    //light_gray_terracotta
    [340] = 0,                                    //cyan_terracotta
    [341] = 0,                                    //purple_terracotta
    [342] = 0,                                    //blue_terracotta
    [343] = 0,                                    //brown_terracotta
    [344] = 0,                                    //green_terracotta
    [345] = 0,                                    //red_terracotta
    [346] = 0,                                    //black_terracotta
    [347] = 0,                                    //barrier
    [348] = I_TDOOR | I_FACINGNESW,               //iron_trapdoor
    [349] = I_AXIS,                               //hay_block
    [350] = 0,                                    //white_carpet
    [351] = 0,                                    //orange_carpet
    [352] = 0,                                    //magenta_carpet
    [353] = 0,                                    //light_blue_carpet
    [354] = 0,                                    //yellow_carpet
    [355] = 0,                                    //lime_carpet
    [356] = 0,                                    //pink_carpet
    [357] = 0,                                    //gray_carpet
    [358] = 0,                                    //light_gray_carpet
    [359] = 0,                                    //cyan_carpet
    [360] = 0,                                    //purple_carpet
    [361] = 0,                                    //blue_carpet
    [362] = 0,                                    //brown_carpet
    [363] = 0,                                    //green_carpet
    [364] = 0,                                    //red_carpet
    [365] = 0,                                    //black_carpet
    [366] = 0,                                    //terracotta
    [367] = 0,                                    //coal_block
    [368] = 0,                                    //packed_ice
    [369] = I_STAIR | I_FACINGNESW,               //acacia_stairs
    [370] = I_STAIR | I_FACINGNESW,               //dark_oak_stairs
    [371] = 0,                                    //slime_block
    [372] = 0,                                    //grass_path
    [373] = 0,                                    //sunflower
    [374] = 0,                                    //lilac
    [375] = 0,                                    //rose_bush
    [376] = 0,                                    //peony
    [377] = 0,                                    //tall_grass
    [378] = 0,                                    //large_fern
    [379] = 0,                                    //white_stained_glass
    [380] = 0,                                    //orange_stained_glass
    [381] = 0,                                    //magenta_stained_glass
    [382] = 0,                                    //light_blue_stained_glass
    [383] = 0,                                    //yellow_stained_glass
    [384] = 0,                                    //lime_stained_glass
    [385] = 0,                                    //pink_stained_glass
    [386] = 0,                                    //gray_stained_glass
    [387] = 0,                                    //light_gray_stained_glass
    [388] = 0,                                    //cyan_stained_glass
    [389] = 0,                                    //purple_stained_glass
    [390] = 0,                                    //blue_stained_glass
    [391] = 0,                                    //brown_stained_glass
    [392] = 0,                                    //green_stained_glass
    [393] = 0,                                    //red_stained_glass
    [394] = 0,                                    //black_stained_glass
    [395] = 0,                                    //white_stained_glass_pane
    [396] = 0,                                    //orange_stained_glass_pane
    [397] = 0,                                    //magenta_stained_glass_pane
    [398] = 0,                                    //light_blue_stained_glass_pane
    [399] = 0,                                    //yellow_stained_glass_pane
    [400] = 0,                                    //lime_stained_glass_pane
    [401] = 0,                                    //pink_stained_glass_pane
    [402] = 0,                                    //gray_stained_glass_pane
    [403] = 0,                                    //light_gray_stained_glass_pane
    [404] = 0,                                    //cyan_stained_glass_pane
    [405] = 0,                                    //purple_stained_glass_pane
    [406] = 0,                                    //blue_stained_glass_pane
    [407] = 0,                                    //brown_stained_glass_pane
    [408] = 0,                                    //green_stained_glass_pane
    [409] = 0,                                    //red_stained_glass_pane
    [410] = 0,                                    //black_stained_glass_pane
    [411] = 0,                                    //prismarine
    [412] = 0,                                    //prismarine_bricks
    [413] = 0,                                    //dark_prismarine
    [414] = I_STAIR | I_FACINGNESW,               //prismarine_stairs
    [415] = I_STAIR | I_FACINGNESW,               //prismarine_brick_stairs
    [416] = I_STAIR | I_FACINGNESW,               //dark_prismarine_stairs
    [417] = 0,                                    //sea_lantern
    [418] = 0,                                    //red_sandstone
    [419] = 0,                                    //chiseled_red_sandstone
    [420] = 0,                                    //cut_red_sandstone
    [421] = I_STAIR | I_FACINGNESW,               //red_sandstone_stairs
    [422] = I_CONT | I_FACINGNESWUD,              //repeating_command_block
    [423] = I_CONT | I_FACINGNESWUD,              //chain_command_block
    [424] = 0,                                    //magma_block
    [425] = 0,                                    //nether_wart_block
    [426] = 0,                                    //warped_wart_block
    [427] = 0,                                    //red_nether_bricks
    [428] = I_AXIS,                               //bone_block
    [429] = 0,                                    //structure_void
    [430] = I_RSDEV | I_FACINGNESWUD,             //observer
    [431] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //shulker_box
    [432] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //white_shulker_box
    [433] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //orange_shulker_box
    [434] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //magenta_shulker_box
    [435] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //light_blue_shulker_box
    [436] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //yellow_shulker_box
    [437] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //lime_shulker_box
    [438] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //pink_shulker_box
    [439] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //gray_shulker_box
    [440] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //light_gray_shulker_box
    [441] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //cyan_shulker_box
    [442] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //purple_shulker_box
    [443] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //blue_shulker_box
    [444] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //brown_shulker_box
    [445] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //green_shulker_box
    [446] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //red_shulker_box
    [447] = I_NSTACK | I_CONT | I_FACINGNESWUD,   //black_shulker_box
    [448] = I_FACINGNESW,                         //white_glazed_terracotta
    [449] = I_FACINGNESW,                         //orange_glazed_terracotta
    [450] = I_FACINGNESW,                         //magenta_glazed_terracotta
    [451] = I_FACINGNESW,                         //light_blue_glazed_terracotta
    [452] = I_FACINGNESW,                         //yellow_glazed_terracotta
    [453] = I_FACINGNESW,                         //lime_glazed_terracotta
    [454] = I_FACINGNESW,                         //pink_glazed_terracotta
    [455] = I_FACINGNESW,                         //gray_glazed_terracotta
    [456] = I_FACINGNESW,                         //light_gray_glazed_terracotta
    [457] = I_FACINGNESW,                         //cyan_glazed_terracotta
    [458] = I_FACINGNESW,                         //purple_glazed_terracotta
    [459] = I_FACINGNESW,                         //blue_glazed_terracotta
    [460] = I_FACINGNESW,                         //brown_glazed_terracotta
    [461] = I_FACINGNESW,                         //green_glazed_terracotta
    [462] = I_FACINGNESW,                         //red_glazed_terracotta
    [463] = I_FACINGNESW,                         //black_glazed_terracotta
    [464] = 0,                                    //white_concrete
    [465] = 0,                                    //orange_concrete
    [466] = 0,                                    //magenta_concrete
    [467] = 0,                                    //light_blue_concrete
    [468] = 0,                                    //yellow_concrete
    [469] = 0,                                    //lime_concrete
    [470] = 0,                                    //pink_concrete
    [471] = 0,                                    //gray_concrete
    [472] = 0,                                    //light_gray_concrete
    [473] = 0,                                    //cyan_concrete
    [474] = 0,                                    //purple_concrete
    [475] = 0,                                    //blue_concrete
    [476] = 0,                                    //brown_concrete
    [477] = 0,                                    //green_concrete
    [478] = 0,                                    //red_concrete
    [479] = 0,                                    //black_concrete
    [480] = 0,                                    //white_concrete_powder
    [481] = 0,                                    //orange_concrete_powder
    [482] = 0,                                    //magenta_concrete_powder
    [483] = 0,                                    //light_blue_concrete_powder
    [484] = 0,                                    //yellow_concrete_powder
    [485] = 0,                                    //lime_concrete_powder
    [486] = 0,                                    //pink_concrete_powder
    [487] = 0,                                    //gray_concrete_powder
    [488] = 0,                                    //light_gray_concrete_powder
    [489] = 0,                                    //cyan_concrete_powder
    [490] = 0,                                    //purple_concrete_powder
    [491] = 0,                                    //blue_concrete_powder
    [492] = 0,                                    //brown_concrete_powder
    [493] = 0,                                    //green_concrete_powder
    [494] = 0,                                    //red_concrete_powder
    [495] = 0,                                    //black_concrete_powder
    [496] = 0,                                    //turtle_egg
    [497] = 0,                                    //dead_tube_coral_block
    [498] = 0,                                    //dead_brain_coral_block
    [499] = 0,                                    //dead_bubble_coral_block
    [500] = 0,                                    //dead_fire_coral_block
    [501] = 0,                                    //dead_horn_coral_block
    [502] = 0,                                    //tube_coral_block
    [503] = 0,                                    //brain_coral_block
    [504] = 0,                                    //bubble_coral_block
    [505] = 0,                                    //fire_coral_block
    [506] = 0,                                    //horn_coral_block
    [507] = 0,                                    //tube_coral
    [508] = 0,                                    //brain_coral
    [509] = 0,                                    //bubble_coral
    [510] = 0,                                    //fire_coral
    [511] = 0,                                    //horn_coral
    [512] = 0,                                    //dead_brain_coral
    [513] = 0,                                    //dead_bubble_coral
    [514] = 0,                                    //dead_fire_coral
    [515] = 0,                                    //dead_horn_coral
    [516] = 0,                                    //dead_tube_coral
    [517] = 0,                                    //tube_coral_fan
    [518] = 0,                                    //brain_coral_fan
    [519] = 0,                                    //bubble_coral_fan
    [520] = 0,                                    //fire_coral_fan
    [521] = 0,                                    //horn_coral_fan
    [522] = 0,                                    //dead_tube_coral_fan
    [523] = 0,                                    //dead_brain_coral_fan
    [524] = 0,                                    //dead_bubble_coral_fan
    [525] = 0,                                    //dead_fire_coral_fan
    [526] = 0,                                    //dead_horn_coral_fan
    [527] = 0,                                    //blue_ice
    [528] = 0,                                    //conduit
    [529] = I_STAIR | I_FACINGNESW,               //polished_granite_stairs
    [530] = I_STAIR | I_FACINGNESW,               //smooth_red_sandstone_stairs
    [531] = I_STAIR | I_FACINGNESW,               //mossy_stone_brick_stairs
    [532] = I_STAIR | I_FACINGNESW,               //polished_diorite_stairs
    [533] = I_STAIR | I_FACINGNESW,               //mossy_cobblestone_stairs
    [534] = I_STAIR | I_FACINGNESW,               //end_stone_brick_stairs
    [535] = I_STAIR | I_FACINGNESW,               //stone_stairs
    [536] = I_STAIR | I_FACINGNESW,               //smooth_sandstone_stairs
    [537] = I_STAIR | I_FACINGNESW,               //smooth_quartz_stairs
    [538] = I_STAIR | I_FACINGNESW,               //granite_stairs
    [539] = I_STAIR | I_FACINGNESW,               //andesite_stairs
    [540] = I_STAIR | I_FACINGNESW,               //red_nether_brick_stairs
    [541] = I_STAIR | I_FACINGNESW,               //polished_andesite_stairs
    [542] = I_STAIR | I_FACINGNESW,               //diorite_stairs
    [543] = I_SLAB,                               //polished_granite_slab
    [544] = I_SLAB,                               //smooth_red_sandstone_slab
    [545] = I_SLAB,                               //mossy_stone_brick_slab
    [546] = I_SLAB,                               //polished_diorite_slab
    [547] = I_SLAB,                               //mossy_cobblestone_slab
    [548] = I_SLAB,                               //end_stone_brick_slab
    [549] = I_SLAB,                               //smooth_sandstone_slab
    [550] = I_SLAB,                               //smooth_quartz_slab
    [551] = I_SLAB,                               //granite_slab
    [552] = I_SLAB,                               //andesite_slab
    [553] = I_SLAB,                               //red_nether_brick_slab
    [554] = I_SLAB,                               //polished_andesite_slab
    [555] = I_SLAB,                               //diorite_slab
    [556] = I_SLAB,                               //scaffolding
    [557] = I_DOOR | I_FACINGNESW,                //iron_door
    [558] = I_DOOR | I_FACINGNESW,                //oak_door
    [559] = I_DOOR | I_FACINGNESW,                //spruce_door
    [560] = I_DOOR | I_FACINGNESW,                //birch_door
    [561] = I_DOOR | I_FACINGNESW,                //jungle_door
    [562] = I_DOOR | I_FACINGNESW,                //acacia_door
    [563] = I_DOOR | I_FACINGNESW,                //dark_oak_door
    [564] = I_DOOR | I_FACINGNESW,                //crimson_door
    [565] = I_DOOR | I_FACINGNESW,                //warped_door
    [566] = I_FACINGNESW | I_ADJ,                 //repeater
    [567] = I_FACINGNESW | I_ADJ,                 //comparator
    [568] = 0,                                    //structure_block
    [569] = 0,                                    //jigsaw
    [570] = I_NSTACK | I_ITEM | I_ARMOR,          //turtle_helmet
    [571] = I_ITEM,                               //scute
    [572] = I_NSTACK | I_ITEM,                    //flint_and_steel
    [573] = I_ITEM,                               //apple
    [574] = I_NSTACK | I_ITEM,                    //bow
    [575] = I_ITEM,                               //arrow
    [576] = I_ITEM,                               //coal
    [577] = I_ITEM,                               //charcoal
    [578] = I_ITEM,                               //diamond
    [579] = I_ITEM,                               //iron_ingot
    [580] = I_ITEM,                               //gold_ingot
    [581] = I_ITEM,                               //netherite_ingot
    [582] = I_ITEM,                               //netherite_scrap
    [583] = I_NSTACK | I_ITEM,                    //wooden_sword
    [584] = I_NSTACK | I_ITEM,                    //wooden_shovel
    [585] = I_NSTACK | I_ITEM,                    //wooden_pickaxe
    [586] = I_NSTACK | I_ITEM,                    //wooden_axe
    [587] = I_NSTACK | I_ITEM,                    //wooden_hoe
    [588] = I_NSTACK | I_ITEM,                    //stone_sword
    [589] = I_NSTACK | I_ITEM,                    //stone_shovel
    [590] = I_NSTACK | I_ITEM,                    //stone_pickaxe
    [591] = I_NSTACK | I_ITEM,                    //stone_axe
    [592] = I_NSTACK | I_ITEM,                    //stone_hoe
    [593] = I_NSTACK | I_ITEM,                    //golden_sword
    [594] = I_NSTACK | I_ITEM,                    //golden_shovel
    [595] = I_NSTACK | I_ITEM,                    //golden_pickaxe
    [596] = I_NSTACK | I_ITEM,                    //golden_axe
    [597] = I_NSTACK | I_ITEM,                    //golden_hoe
    [598] = I_NSTACK | I_ITEM,                    //iron_sword
    [599] = I_NSTACK | I_ITEM,                    //iron_shovel
    [600] = I_NSTACK | I_ITEM,                    //iron_pickaxe
    [601] = I_NSTACK | I_ITEM,                    //iron_axe
    [602] = I_NSTACK | I_ITEM,                    //iron_hoe
    [603] = I_NSTACK | I_ITEM,                    //diamond_sword
    [604] = I_NSTACK | I_ITEM,                    //diamond_shovel
    [605] = I_NSTACK | I_ITEM,                    //diamond_pickaxe
    [606] = I_NSTACK | I_ITEM,                    //diamond_axe
    [607] = I_NSTACK | I_ITEM,                    //diamond_hoe
    [608] = I_NSTACK | I_ITEM,                    //netherite_sword
    [609] = I_NSTACK | I_ITEM,                    //netherite_shovel
    [610] = I_NSTACK | I_ITEM,                    //netherite_pickaxe
    [611] = I_NSTACK | I_ITEM,                    //netherite_axe
    [612] = I_NSTACK | I_ITEM,                    //netherite_hoe
    [613] = I_ITEM,                               //stick
    [614] = I_ITEM,                               //bowl
    [615] = I_NSTACK | I_ITEM,                    //mushroom_stew
    [616] = I_ITEM,                               //string
    [617] = I_ITEM,                               //feather
    [618] = I_ITEM,                               //gunpowder
    [619] = I_ITEM,                               //wheat_seeds
    [620] = 0,                                    //wheat
    [621] = I_ITEM,                               //bread
    [622] = I_NSTACK | I_ITEM | I_ARMOR,          //leather_helmet
    [623] = I_NSTACK | I_ITEM | I_ARMOR,          //leather_chestplate
    [624] = I_NSTACK | I_ITEM | I_ARMOR,          //leather_leggings
    [625] = I_NSTACK | I_ITEM | I_ARMOR,          //leather_boots
    [626] = I_NSTACK | I_ITEM | I_ARMOR,          //chainmail_helmet
    [627] = I_NSTACK | I_ITEM | I_ARMOR,          //chainmail_chestplate
    [628] = I_NSTACK | I_ITEM | I_ARMOR,          //chainmail_leggings
    [629] = I_NSTACK | I_ITEM | I_ARMOR,          //chainmail_boots
    [630] = I_NSTACK | I_ITEM | I_ARMOR,          //iron_helmet
    [631] = I_NSTACK | I_ITEM | I_ARMOR,          //iron_chestplate
    [632] = I_NSTACK | I_ITEM | I_ARMOR,          //iron_leggings
    [633] = I_NSTACK | I_ITEM | I_ARMOR,          //iron_boots
    [634] = I_NSTACK | I_ITEM | I_ARMOR,          //diamond_helmet
    [635] = I_NSTACK | I_ITEM | I_ARMOR,          //diamond_chestplate
    [636] = I_NSTACK | I_ITEM | I_ARMOR,          //diamond_leggings
    [637] = I_NSTACK | I_ITEM | I_ARMOR,          //diamond_boots
    [638] = I_NSTACK | I_ITEM | I_ARMOR,          //golden_helmet
    [639] = I_NSTACK | I_ITEM | I_ARMOR,          //golden_chestplate
    [640] = I_NSTACK | I_ITEM | I_ARMOR,          //golden_leggings
    [641] = I_NSTACK | I_ITEM | I_ARMOR,          //golden_boots
    [642] = I_NSTACK | I_ITEM | I_ARMOR,          //netherite_helmet
    [643] = I_NSTACK | I_ITEM | I_ARMOR,          //netherite_chestplate
    [644] = I_NSTACK | I_ITEM | I_ARMOR,          //netherite_leggings
    [645] = I_NSTACK | I_ITEM | I_ARMOR,          //netherite_boots
    [646] = I_ITEM,                               //flint
    [647] = I_ITEM,                               //porkchop
    [648] = I_ITEM,                               //cooked_porkchop
    [649] = I_ITEM,                               //painting
    [650] = I_ITEM,                               //golden_apple
    [651] = I_ITEM,                               //enchanted_golden_apple
    [652] = I_S16,                                //oak_sign
    [653] = I_S16,                                //spruce_sign
    [654] = I_S16,                                //birch_sign
    [655] = I_S16,                                //jungle_sign
    [656] = I_S16,                                //acacia_sign
    [657] = I_S16,                                //dark_oak_sign
    [658] = I_S16,                                //crimson_sign
    [659] = I_S16,                                //warped_sign
    [660] = I_S16 | I_ITEM,                       //bucket
    [661] = I_NSTACK | I_ITEM,                    //water_bucket
    [662] = I_NSTACK | I_ITEM,                    //lava_bucket
    [663] = I_NSTACK | I_ITEM,                    //minecart
    [664] = I_NSTACK | I_ITEM,                    //saddle
    [665] = I_ITEM,                               //redstone
    [666] = I_S16 | I_ITEM,                       //snowball
    [667] = I_NSTACK | I_ITEM,                    //oak_boat
    [668] = I_ITEM,                               //leather
    [669] = I_NSTACK | I_ITEM,                    //milk_bucket
    [670] = I_NSTACK | I_ITEM,                    //pufferfish_bucket
    [671] = I_NSTACK | I_ITEM,                    //salmon_bucket
    [672] = I_NSTACK | I_ITEM,                    //cod_bucket
    [673] = I_NSTACK | I_ITEM,                    //tropical_fish_bucket
    [674] = I_ITEM,                               //brick
    [675] = I_ITEM,                               //clay_ball
    [676] = 0,                                    //dried_kelp_block
    [677] = I_ITEM,                               //paper
    [678] = I_ITEM,                               //book
    [679] = I_ITEM,                               //slime_ball
    [680] = I_NSTACK | I_ITEM,                    //chest_minecart
    [681] = I_NSTACK | I_ITEM,                    //furnace_minecart
    [682] = I_S16 | I_ITEM,                       //egg
    [683] = I_ITEM,                               //compass
    [684] = I_NSTACK | I_ITEM,                    //fishing_rod
    [685] = I_ITEM,                               //clock
    [686] = I_ITEM,                               //glowstone_dust
    [687] = I_ITEM,                               //cod
    [688] = I_ITEM,                               //salmon
    [689] = I_ITEM,                               //tropical_fish
    [690] = I_ITEM,                               //pufferfish
    [691] = I_ITEM,                               //cooked_cod
    [692] = I_ITEM,                               //cooked_salmon
    [693] = I_ITEM,                               //ink_sac
    [694] = I_ITEM,                               //cocoa_beans
    [695] = I_ITEM,                               //lapis_lazuli
    [696] = I_ITEM,                               //white_dye
    [697] = I_ITEM,                               //orange_dye
    [698] = I_ITEM,                               //magenta_dye
    [699] = I_ITEM,                               //light_blue_dye
    [700] = I_ITEM,                               //yellow_dye
    [701] = I_ITEM,                               //lime_dye
    [702] = I_ITEM,                               //pink_dye
    [703] = I_ITEM,                               //gray_dye
    [704] = I_ITEM,                               //light_gray_dye
    [705] = I_ITEM,                               //cyan_dye
    [706] = I_ITEM,                               //purple_dye
    [707] = I_ITEM,                               //blue_dye
    [708] = I_ITEM,                               //brown_dye
    [709] = I_ITEM,                               //green_dye
    [710] = I_ITEM,                               //red_dye
    [711] = I_ITEM,                               //black_dye
    [712] = I_ITEM,                               //bone_meal
    [713] = I_ITEM,                               //bone
    [714] = I_ITEM,                               //sugar
    [715] = I_NSTACK,                             //cake
    [716] = I_NSTACK | I_BED | I_FACINGNESW,      //white_bed
    [717] = I_NSTACK | I_BED | I_FACINGNESW,      //orange_bed
    [718] = I_NSTACK | I_BED | I_FACINGNESW,      //magenta_bed
    [719] = I_NSTACK | I_BED | I_FACINGNESW,      //light_blue_bed
    [720] = I_NSTACK | I_BED | I_FACINGNESW,      //yellow_bed
    [721] = I_NSTACK | I_BED | I_FACINGNESW,      //lime_bed
    [722] = I_NSTACK | I_BED | I_FACINGNESW,      //pink_bed
    [723] = I_NSTACK | I_BED | I_FACINGNESW,      //gray_bed
    [724] = I_NSTACK | I_BED | I_FACINGNESW,      //light_gray_bed
    [725] = I_NSTACK | I_BED | I_FACINGNESW,      //cyan_bed
    [726] = I_NSTACK | I_BED | I_FACINGNESW,      //purple_bed
    [727] = I_NSTACK | I_BED | I_FACINGNESW,      //blue_bed
    [728] = I_NSTACK | I_BED | I_FACINGNESW,      //brown_bed
    [729] = I_NSTACK | I_BED | I_FACINGNESW,      //green_bed
    [730] = I_NSTACK | I_BED | I_FACINGNESW,      //red_bed
    [731] = I_NSTACK | I_BED | I_FACINGNESW,      //black_bed
    [732] = I_ITEM,                               //cookie
    [733] = I_ITEM,                               //filled_map
    [734] = I_NSTACK | I_ITEM,                    //shears
    [735] = I_ITEM,                               //melon_slice
    [736] = I_ITEM,                               //dried_kelp
    [737] = I_ITEM,                               //pumpkin_seeds
    [738] = I_ITEM,                               //melon_seeds
    [739] = I_ITEM,                               //beef
    [740] = I_ITEM,                               //cooked_beef
    [741] = I_ITEM,                               //chicken
    [742] = I_ITEM,                               //cooked_chicken
    [743] = I_ITEM,                               //rotten_flesh
    [744] = I_S16 | I_ITEM,                       //ender_pearl
    [745] = I_ITEM,                               //blaze_rod
    [746] = I_ITEM,                               //ghast_tear
    [747] = I_ITEM,                               //gold_nugget
    [748] = 0,                                    //nether_wart
    [749] = I_NSTACK | I_ITEM,                    //potion
    [750] = I_ITEM,                               //glass_bottle
    [751] = I_ITEM,                               //spider_eye
    [752] = I_ITEM,                               //fermented_spider_eye
    [753] = I_ITEM,                               //blaze_powder
    [754] = I_ITEM,                               //magma_cream
    [755] = I_CONT,                               //brewing_stand
    [756] = 0,                                    //cauldron
    [757] = I_ITEM,                               //ender_eye
    [758] = I_ITEM,                               //glistering_melon_slice
    [759] = I_ITEM,                               //bat_spawn_egg
    [760] = I_ITEM,                               //bee_spawn_egg
    [761] = I_ITEM,                               //blaze_spawn_egg
    [762] = I_ITEM,                               //cat_spawn_egg
    [763] = I_ITEM,                               //cave_spider_spawn_egg
    [764] = I_ITEM,                               //chicken_spawn_egg
    [765] = I_ITEM,                               //cod_spawn_egg
    [766] = I_ITEM,                               //cow_spawn_egg
    [767] = I_ITEM,                               //creeper_spawn_egg
    [768] = I_ITEM,                               //dolphin_spawn_egg
    [769] = I_ITEM,                               //donkey_spawn_egg
    [770] = I_ITEM,                               //drowned_spawn_egg
    [771] = I_ITEM,                               //elder_guardian_spawn_egg
    [772] = I_ITEM,                               //enderman_spawn_egg
    [773] = I_ITEM,                               //endermite_spawn_egg
    [774] = I_ITEM,                               //evoker_spawn_egg
    [775] = I_ITEM,                               //fox_spawn_egg
    [776] = I_ITEM,                               //ghast_spawn_egg
    [777] = I_ITEM,                               //guardian_spawn_egg
    [778] = I_ITEM,                               //hoglin_spawn_egg
    [779] = I_ITEM,                               //horse_spawn_egg
    [780] = I_ITEM,                               //husk_spawn_egg
    [781] = I_ITEM,                               //llama_spawn_egg
    [782] = I_ITEM,                               //magma_cube_spawn_egg
    [783] = I_ITEM,                               //mooshroom_spawn_egg
    [784] = I_ITEM,                               //mule_spawn_egg
    [785] = I_ITEM,                               //ocelot_spawn_egg
    [786] = I_ITEM,                               //panda_spawn_egg
    [787] = I_ITEM,                               //parrot_spawn_egg
    [788] = I_ITEM,                               //phantom_spawn_egg
    [789] = I_ITEM,                               //pig_spawn_egg
    [790] = I_ITEM,                               //piglin_spawn_egg
    [791] = I_ITEM,                               //piglin_brute_spawn_egg
    [792] = I_ITEM,                               //pillager_spawn_egg
    [793] = I_ITEM,                               //polar_bear_spawn_egg
    [794] = I_ITEM,                               //pufferfish_spawn_egg
    [795] = I_ITEM,                               //rabbit_spawn_egg
    [796] = I_ITEM,                               //ravager_spawn_egg
    [797] = I_ITEM,                               //salmon_spawn_egg
    [798] = I_ITEM,                               //sheep_spawn_egg
    [799] = I_ITEM,                               //shulker_spawn_egg
    [800] = I_ITEM,                               //silverfish_spawn_egg
    [801] = I_ITEM,                               //skeleton_spawn_egg
    [802] = I_ITEM,                               //skeleton_horse_spawn_egg
    [803] = I_ITEM,                               //slime_spawn_egg
    [804] = I_ITEM,                               //spider_spawn_egg
    [805] = I_ITEM,                               //squid_spawn_egg
    [806] = I_ITEM,                               //stray_spawn_egg
    [807] = I_ITEM,                               //strider_spawn_egg
    [808] = I_ITEM,                               //trader_llama_spawn_egg
    [809] = I_ITEM,                               //tropical_fish_spawn_egg
    [810] = I_ITEM,                               //turtle_spawn_egg
    [811] = I_ITEM,                               //vex_spawn_egg
    [812] = I_ITEM,                               //villager_spawn_egg
    [813] = I_ITEM,                               //vindicator_spawn_egg
    [814] = I_ITEM,                               //wandering_trader_spawn_egg
    [815] = I_ITEM,                               //witch_spawn_egg
    [816] = I_ITEM,                               //wither_skeleton_spawn_egg
    [817] = I_ITEM,                               //wolf_spawn_egg
    [818] = I_ITEM,                               //zoglin_spawn_egg
    [819] = I_ITEM,                               //zombie_spawn_egg
    [820] = I_ITEM,                               //zombie_horse_spawn_egg
    [821] = I_ITEM,                               //zombie_villager_spawn_egg
    [822] = I_ITEM,                               //zombified_piglin_spawn_egg
    [823] = I_ITEM,                               //experience_bottle
    [824] = I_ITEM,                               //fire_charge
    [825] = I_NSTACK | I_ITEM,                    //writable_book
    [826] = I_S16 | I_ITEM,                       //written_book
    [827] = I_ITEM,                               //emerald
    [828] = I_ITEM,                               //item_frame
    [829] = 0,                                    //flower_pot
    [830] = I_ITEM,                               //carrot
    [831] = I_ITEM,                               //potato
    [832] = I_ITEM,                               //baked_potato
    [833] = I_ITEM,                               //poisonous_potato
    [834] = I_ITEM,                               //map
    [835] = I_ITEM,                               //golden_carrot
    [836] = 0,                                    //skeleton_skull
    [837] = 0,                                    //wither_skeleton_skull
    [838] = 0,                                    //player_head
    [839] = 0,                                    //zombie_head
    [840] = 0,                                    //creeper_head
    [841] = 0,                                    //dragon_head
    [842] = I_NSTACK | I_ITEM,                    //carrot_on_a_stick
    [843] = I_NSTACK | I_ITEM,                    //warped_fungus_on_a_stick
    [844] = I_ITEM,                               //nether_star
    [845] = I_ITEM,                               //pumpkin_pie
    [846] = I_ITEM,                               //firework_rocket
    [847] = I_ITEM,                               //firework_star
    [848] = I_NSTACK | I_ITEM,                    //enchanted_book
    [849] = I_ITEM,                               //nether_brick
    [850] = I_ITEM,                               //quartz
    [851] = I_NSTACK | I_ITEM,                    //tnt_minecart
    [852] = I_NSTACK | I_ITEM,                    //hopper_minecart
    [853] = I_ITEM,                               //prismarine_shard
    [854] = I_ITEM,                               //prismarine_crystals
    [855] = I_ITEM,                               //rabbit
    [856] = I_ITEM,                               //cooked_rabbit
    [857] = I_NSTACK | I_ITEM,                    //rabbit_stew
    [858] = I_ITEM,                               //rabbit_foot
    [859] = I_ITEM,                               //rabbit_hide
    [860] = I_S16 | I_ITEM,                       //armor_stand
    [861] = I_NSTACK | I_ITEM,                    //iron_horse_armor
    [862] = I_NSTACK | I_ITEM,                    //golden_horse_armor
    [863] = I_NSTACK | I_ITEM,                    //diamond_horse_armor
    [864] = I_NSTACK | I_ITEM,                    //leather_horse_armor
    [865] = I_ITEM,                               //lead
    [866] = I_ITEM,                               //name_tag
    [867] = I_NSTACK | I_ITEM,                    //command_block_minecart
    [868] = I_ITEM,                               //mutton
    [869] = I_ITEM,                               //cooked_mutton
    [870] = I_S16,                                //white_banner
    [871] = I_S16,                                //orange_banner
    [872] = I_S16,                                //magenta_banner
    [873] = I_S16,                                //light_blue_banner
    [874] = I_S16,                                //yellow_banner
    [875] = I_S16,                                //lime_banner
    [876] = I_S16,                                //pink_banner
    [877] = I_S16,                                //gray_banner
    [878] = I_S16,                                //light_gray_banner
    [879] = I_S16,                                //cyan_banner
    [880] = I_S16,                                //purple_banner
    [881] = I_S16,                                //blue_banner
    [882] = I_S16,                                //brown_banner
    [883] = I_S16,                                //green_banner
    [884] = I_S16,                                //red_banner
    [885] = I_S16,                                //black_banner
    [886] = I_ITEM,                               //end_crystal
    [887] = I_ITEM,                               //chorus_fruit
    [888] = I_ITEM,                               //popped_chorus_fruit
    [889] = I_ITEM,                               //beetroot
    [890] = I_ITEM,                               //beetroot_seeds
    [891] = I_NSTACK | I_ITEM,                    //beetroot_soup
    [892] = I_ITEM,                               //dragon_breath
    [893] = I_NSTACK | I_ITEM,                    //splash_potion
    [894] = I_NSTACK | I_ITEM,                    //spectral_arrow
    [895] = I_NSTACK | I_ITEM,                    //tipped_arrow
    [896] = I_NSTACK | I_ITEM,                    //lingering_potion
    [897] = I_NSTACK | I_ITEM | I_ARMOR,          //shield
    [898] = I_NSTACK | I_ITEM | I_ARMOR,          //elytra
    [899] = I_NSTACK | I_ITEM,                    //spruce_boat
    [900] = I_NSTACK | I_ITEM,                    //birch_boat
    [901] = I_NSTACK | I_ITEM,                    //jungle_boat
    [902] = I_NSTACK | I_ITEM,                    //acacia_boat
    [903] = I_NSTACK | I_ITEM,                    //dark_oak_boat
    [904] = I_NSTACK | I_ITEM,                    //totem_of_undying
    [905] = I_ITEM,                               //shulker_shell
    [906] = I_ITEM,                               //iron_nugget
    [907] = I_NSTACK | I_ITEM,                    //knowledge_book
    [908] = I_NSTACK | I_ITEM,                    //debug_stick
    [909] = I_NSTACK | I_ITEM,                    //music_disc_13
    [910] = I_NSTACK | I_ITEM,                    //music_disc_cat
    [911] = I_NSTACK | I_ITEM,                    //music_disc_blocks
    [912] = I_NSTACK | I_ITEM,                    //music_disc_chirp
    [913] = I_NSTACK | I_ITEM,                    //music_disc_far
    [914] = I_NSTACK | I_ITEM,                    //music_disc_mall
    [915] = I_NSTACK | I_ITEM,                    //music_disc_mellohi
    [916] = I_NSTACK | I_ITEM,                    //music_disc_stal
    [917] = I_NSTACK | I_ITEM,                    //music_disc_strad
    [918] = I_NSTACK | I_ITEM,                    //music_disc_ward
    [919] = I_NSTACK | I_ITEM,                    //music_disc_11
    [920] = I_NSTACK | I_ITEM,                    //music_disc_wait
    [921] = I_NSTACK | I_ITEM,                    //music_disc_pigstep
    [922] = I_NSTACK | I_ITEM,                    //trident
    [923] = I_ITEM,                               //phantom_membrane
    [924] = I_ITEM,                               //nautilus_shell
    [925] = I_ITEM,                               //heart_of_the_sea
    [926] = I_NSTACK | I_ITEM,                    //crossbow
    [927] = I_NSTACK | I_ITEM,                    //suspicious_stew
    [928] = I_CONT,                               //loom
    [929] = I_NSTACK,                             //flower_banner_pattern
    [930] = I_NSTACK,                             //creeper_banner_pattern
    [931] = I_NSTACK,                             //skull_banner_pattern
    [932] = I_NSTACK,                             //mojang_banner_pattern
    [933] = I_NSTACK,                             //globe_banner_pattern
    [934] = I_NSTACK,                             //piglin_banner_pattern
    [935] = I_CONT,                               //composter
    [936] = I_CONT,                               //barrel
    [937] = I_CONT,                               //smoker
    [938] = I_CONT,                               //blast_furnace
    [939] = I_CONT,                               //cartography_table
    [940] = I_CONT,                               //fletching_table
    [941] = I_CONT,                               //grindstone
    [942] = I_CONT,                               //lectern
    [943] = I_CONT,                               //smithing_table
    [944] = I_CONT,                               //stonecutter
    [945] = I_NSTACK,                             //bell
    [946] = 0,                                    //lantern
    [947] = 0,                                    //soul_lantern
    [948] = 0,                                    //sweet_berries
    [949] = 0,                                    //campfire
    [950] = 0,                                    //soul_campfire
    [951] = 0,                                    //shroomlight
    [952] = 0,                                    //honeycomb
    [953] = 0,                                    //bee_nest
    [954] = 0,                                    //beehive
    [955] = I_S16,                                //honey_bottle
    [956] = 0,                                    //honey_block
    [957] = 0,                                    //honeycomb_block
    [958] = 0,                                    //lodestone
    [959] = 0,                                    //netherite_block
    [960] = 0,                                    //ancient_debris
    [961] = 0,                                    //target
    [962] = 0,                                    //crying_obsidian
    [963] = 0,                                    //blackstone
    [964] = I_SLAB,                               //blackstone_slab
    [965] = I_STAIR | I_FACINGNESW,               //blackstone_stairs
    [966] = 0,                                    //gilded_blackstone
    [967] = 0,                                    //polished_blackstone
    [968] = I_SLAB,                               //polished_blackstone_slab
    [969] = I_STAIR | I_FACINGNESW,               //polished_blackstone_stairs
    [970] = 0,                                    //chiseled_polished_blackstone
    [971] = 0,                                    //polished_blackstone_bricks
    [972] = I_SLAB,                               //polished_blackstone_brick_slab
    [973] = I_STAIR | I_FACINGNESW,               //polished_blackstone_brick_stairs
    [974] = 0,                                    //cracked_polished_blackstone_bricks
    [975] = 0,                                    //respawn_anchor

};

const int db_num_items = sizeof(item_flags)/sizeof(uint64_t);

// Returns stacksize of an item
int db_stacksize (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_NSTACK) {
        return 1; //doesnt stack
    }
    if (item_flags[item_id] & I_S16) {
        return 16;
    }
    return 64;
}

// True if item exists as item only
int db_item_is_itemonly (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_ITEM) {
        return 1;
    }
    return 0;
}

// True if item is a container (opens a dialog window)
int db_item_is_container (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_CONT) {
        return 1;
    }
    return 0;
}

// True if item is placed on an axis (logs, wood, stripped, quartz & purper pillars, hay & bone)
int db_item_is_axis (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_AXIS) {
        return 1;
    }
    return 0;
}

// True if item is a slab
int db_item_is_slab (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_SLAB) {
        return 1;
    }
    return 0;
}

// True if item is a stair
int db_item_is_stair (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_STAIR) {
        return 1;
    }
    return 0;
}
// True if item is a door
int db_item_is_door (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_DOOR) {
        return 1;
    }
    return 0;
}

// True if item is a trapdoor
int db_item_is_tdoor (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_TDOOR) {
        return 1;
    }
    return 0;
}

// True if item has the face property (buttons, lever, upcoming grindstone)
int db_item_is_face (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_FACE) {
        return 1;
    }
    return 0;
}

// True if item is a bed
int db_item_is_bed (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_BED) {
        return 1;
    }
    return 0;
}

// True if item is a redstone device
int db_item_is_rsdev (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_RSDEV) {
        return 1;
    }
    return 0;
}

// True if item is a chest or trapped chest (single/left/right orientable containers)
int db_item_is_chest (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_CHEST) {
        return 1;
    }
    return 0;
}

// True if item is a enderchest or furnace  (orientable containers)
int db_item_is_furnace (int item_id) {
    assert ( item_id >= 0 && item_id < db_num_items );
    if (item_flags[item_id] & I_FURNACE) {
        return 1;
    }
    return 0;
}
