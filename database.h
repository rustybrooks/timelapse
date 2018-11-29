#pragma once

#include "log.h"
#include "ev.h"
#include "timer.h"

#include "sqlite3.h"

#include <string>
#include <boost/unordered_map.hpp>
#include <map>
#include <list>
#include <sys/time.h>

#include <boost/lexical_cast.hpp>

using namespace std;

int photo_exists_callback(void *data, int argc, char **argv, char **azColName);
int photo_record_callback(void *data, int argc, char **argv, char **azColName);
int preload_db_callback(void *data, int argc, char **argv, char **azColName);

struct PhotoRecord {

    PhotoRecord(sqlite3 *handle=NULL, string filename="", bool load=true) 
        : _handle(handle)
        , _filename(filename)
        , _exposure_time(0)
        , _aperture(0)
        , _measured_ev(0)
        , _iso(0)
    {
        if (!filename.size()) return;
        if (!load) return;

        char *errmsg = 0;
        string sql("select count(*) as count from photos where filename='" + filename + "';");
        int result = sqlite3_exec(_handle, sql.c_str(), photo_exists_callback, (void *) this, &errmsg);
        if (result != SQLITE_OK) {
            debug("SQL select statement failed: %x\n", result);
        }
    }

    void load_from_db() {
        string sql("select photo_id, exposure_time, aperture, iso, measured_ev, time from photos where filename='" + _filename + "';");
        
        char *errmsg = 0;
        int result = sqlite3_exec(_handle, sql.c_str(), photo_record_callback, (void *) this, &errmsg);
        if (result != SQLITE_OK) {
            debug("PhotoRecord SQL select statement failed: %x\n", result);
        }
    }
    
    void load_from_file(bool do_save=true) {
        EXIF ex;
        
        ex.init(_filename);
        _exposure_time = ex.exposure_time();
        _aperture = ex.aperture();
        _measured_ev = ex.measured_ev();
        _iso = ex.iso();
        _time = ex.meta_timestamp();
        
        save();
    }
    
    void save() {
        int sz = _filename.size() + 1000;
        char sql[sz];
        char tmstr[20];
        snprintf(tmstr, 20, "%04d-%02d-%02d %02d:%02d:%02d", _time.tm_year+1900, _time.tm_mon, _time.tm_mday, _time.tm_hour, _time.tm_min, _time.tm_sec);
        snprintf(sql, sz-1, "insert into photos(filename, exposure_time, aperture, iso, measured_ev, time) values ('%s', '%0.6f', '%0.2f', '%d', '%f', '%s')", _filename.c_str(), _exposure_time, _aperture, _iso, _measured_ev, tmstr);

        char *errmsg = 0;
        int result = sqlite3_exec(_handle, sql, NULL, NULL, &errmsg);
        if (result != SQLITE_OK) {
            debug("PhotoRecord SQL insert statement failed: %x\n", result);
        }
    }
    
    string toString() {
        string s("");
        s += "PhotoRecord: id=" +  boost::lexical_cast<string>(_photo_id);;
        s += " file=" + _filename;
        s += " exposure=" + boost::lexical_cast<string>(_exposure_time);
        s += " apeture=" + boost::lexical_cast<string>(_aperture);
        s += " iso=" + boost::lexical_cast<string>(_iso);
        s += " ev=" + boost::lexical_cast<string>(_measured_ev);
        char tmstr[21];
        snprintf(tmstr, 20, "%04d-%02d-%02d %02d:%02d:%02d", _time.tm_year+1900, _time.tm_mon, _time.tm_mday, _time.tm_hour, _time.tm_min, _time.tm_sec);
        s += " time=" + string(tmstr);
        return s;
    }

    sqlite3 *_handle;
    int _photo_id;
    string _filename;
    double _exposure_time, _aperture, _measured_ev;
    int _iso;
    struct tm _time;
};

class PhotoDB {
public:
    PhotoDB(string file=string("")) 
        : _handle(NULL)
    {
        if (file.size()) reopen(file);
    }

    ~PhotoDB() {
        sqlite3_close(_handle);
    }

    void preload() {
        debug("Preloading db...\n");
        char *errmsg = 0;
        string sql("select filename, photo_id, exposure_time, aperture, iso, measured_ev, time from photos order by photo_id;");
        int result = sqlite3_exec(_handle, sql.c_str(), preload_db_callback, (void *) this, &errmsg);
        if (result != SQLITE_OK) {
            debug("SQL select statement failed: %x\n", result);
        }
        debug("Done preloading db...\n");
    }

    void reopen(string file) {
        if (_handle != NULL) sqlite3_close(_handle);

        _file = file;
        sqlite3_open(file.c_str(), &_handle);
        create_tables();
    }
    
    void create_tables() {
        int result;
        char *errmsg = 0;
        
        string sql("create table photos("
                   "photo_id integer primary key,"
                   "filename text,"
                   "exposure_time real,"
                   "aperture real,"
                   "iso integer,"
                   "measured_ev real,"
                   "time timestamp" 
                   ")");
        result = sqlite3_exec(_handle, sql.c_str(), NULL, NULL, &errmsg);
        if (result != SQLITE_OK) {
            debug("SQL create table statement failed: %x\n", result);
        }

        if (errmsg) sqlite3_free(errmsg);
    }
    
    bool exists(string filename) {
        boost::unordered_map<string, const PhotoRecord *>::iterator f = _filemap.find(filename);
        return f != _filemap.end();
    }

    PhotoRecord &add_photo(string filename) {
        _photos.push_back(PhotoRecord(_handle, filename));
        const PhotoRecord r =  _photos.back();
        _filemap.insert(std::pair<string, const PhotoRecord *>(filename, &r));
        return _photos.back();
    }

    size_t size() { return _photos.size(); }

public:
    string _file;
    sqlite3 *_handle;
    list<PhotoRecord> _photos;
    boost::unordered_map<string, const PhotoRecord *> _filemap;
};


int photo_exists_callback(void *data, int argc, char **argv, char **azColName) {
    PhotoRecord *photo = (PhotoRecord *) data;
    if (strcmp(argv[0], "0") == 0) {
        photo->load_from_file();
    } else {
        photo->load_from_db();
    }

    return 0;
}

int photo_record_callback(void *data, int argc, char **argv, char **azColName) {
    PhotoRecord *photo = (PhotoRecord *) data;

    for (int i=0; i<argc; i++) {
        if (strcmp(azColName[i], "photo_id") == 0) {
            photo->_photo_id = boost::lexical_cast<int>(argv[i]);
        } else if (strcmp(azColName[i], "exposure_time") == 0) {
            photo->_exposure_time = boost::lexical_cast<double>(argv[i]);
        } else if (strcmp(azColName[i], "aperture") == 0) {
            photo->_aperture = boost::lexical_cast<double>(argv[i]);
        } else if (strcmp(azColName[i], "iso") == 0) {
            photo->_iso = boost::lexical_cast<double>(argv[i]);
        } else if (strcmp(azColName[i], "measured_ev") == 0) {
            photo->_measured_ev = boost::lexical_cast<double>(argv[i]);
        } else if (strcmp(azColName[i], "time") == 0) {
            strptime(argv[i], "%Y-%m-%d %H%M:%S", &photo->_time);
        }
    }

    return 0;
}

int preload_db_callback(void *data, int argc, char **argv, char **azColName) {
    PhotoDB *db = (PhotoDB *) data;

    db->_photos.push_back(PhotoRecord(db->_handle, argv[0], false));
    db->_filemap.insert(std::make_pair(string(argv[0]), &db->_photos.back()));
    photo_record_callback((void *) &(db->_photos.back()), argc, argv, azColName);

    if (db->_photos.size() % 2000 == 0) debug("%f Reached %lu photos\n", time_now(), db->_photos.size());

    return 0;
}
