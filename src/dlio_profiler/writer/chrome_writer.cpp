//
// Created by haridev on 3/28/23.
//

#include <dlio_profiler/writer/chrome_writer.h>
#include <fcntl.h>
#include <dlio_profiler/core/macro.h>
#include <cassert>
#include <unistd.h>
#include <thread>
#include <sstream>
#include <cmath>

#include <uv.h>
uv_rwlock_t numlock;


void dlio_profiler::ChromeWriter::initialize(char *filename, bool throw_error) {
  DLIO_PROFILER_LOGDEBUG("ChromeWriter.initialize","");
  this->throw_error = throw_error;
  this->filename = filename;
  if (fd == -1) {
    fd = dlp_open(filename, O_WRONLY | O_APPEND | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
      ERROR(fd == -1, "unable to create log file %s", filename); // GCOVR_EXCL_LINE
    } else {
      DLIO_PROFILER_LOGINFO("created log file %s with fd %d", filename, fd);
    }
  }
}

void
dlio_profiler::ChromeWriter::log(ConstEventType event_name, ConstEventType category, TimeResolution &start_time,
                                 TimeResolution &duration,
                                 std::unordered_map<std::string, std::any> *metadata, ProcessID process_id, ThreadID thread_id) {
  DLIO_PROFILER_LOGDEBUG("ChromeWriter.log","");
  if (fd != -1) {
    int size;
    char data[MAX_LINE_SIZE];
    convert_json(event_name, category, start_time, duration, metadata, process_id, thread_id, &size, data);
    merge_buffer(data, size);
  }
  is_first_write = false;
}

void dlio_profiler::ChromeWriter::finalize() {
  DLIO_PROFILER_LOGDEBUG("ChromeWriter.finalize","");
  if (fd != -1) {
    DLIO_PROFILER_LOGINFO("Profiler finalizing writer %s", filename.c_str());
    write_buffer_op();
    free(write_buffer);
    int status = dlp_close(fd);
    if (status != 0) {
      ERROR(status != 0, "unable to close log file %d for a+", filename.c_str());  // GCOVR_EXCL_LINE
    }
    if (index == 0) {
      DLIO_PROFILER_LOGINFO("No trace data written. Deleting file %s", filename.c_str());
      dlp_unlink(filename.c_str());
    } else {
      DLIO_PROFILER_LOGINFO("Profiler writing the final symbol", "");
      fd = dlp_open(this->filename.c_str(), O_WRONLY);
      if (fd == -1) {
        ERROR(fd == -1, "unable to open log file %s with O_WRONLY", this->filename.c_str());  // GCOVR_EXCL_LINE
      }
      std::string data = "[\n";
      auto written_elements = dlp_write(fd, data.c_str(), data.size());
      if (written_elements != data.size()) {  // GCOVR_EXCL_START
        ERROR(written_elements != data.size(), "unable to finalize log write %s for O_WRONLY written only %d of %d",
              filename.c_str(), data.size(), written_elements);
      } // GCOVR_EXCL_STOP
      status = dlp_close(fd);
      if (status != 0) {
        ERROR(status != 0, "unable to close log file %d for O_WRONLY", filename.c_str());  // GCOVR_EXCL_LINE
      }
      if (enable_compression) {
        if (system("which gzip > /dev/null 2>&1")) {
          DLIO_PROFILER_LOGERROR("Gzip compression does not exists", "");  // GCOVR_EXCL_LINE
        } else {
          DLIO_PROFILER_LOGINFO("Applying Gzip compression on file %s", filename.c_str());
          char cmd[2048];
          sprintf(cmd, "gzip -f %s", filename.c_str());
          int ret = system(cmd);
          if (ret == 0) {
            DLIO_PROFILER_LOGINFO("Successfully compressed file %s.gz", filename.c_str());
          } else
            DLIO_PROFILER_LOGERROR("Unable to compress file %s", filename.c_str());
        }
      }
    }
  }
  if (enable_core_affinity) {
    hwloc_topology_destroy(topology);
  }
  DLIO_PROFILER_LOGDEBUG("Finished writer finalization", "");
}


void
dlio_profiler::ChromeWriter::convert_json(ConstEventType event_name, ConstEventType category, TimeResolution start_time,
                                          TimeResolution duration, std::unordered_map<std::string, std::any> *metadata,
                                          ProcessID process_id, ThreadID thread_id, int* size, char* data) {
  DLIO_PROFILER_LOGDEBUG("ChromeWriter.convert_json","");

  std::string is_first_char = "";
  if (is_first_write) is_first_char = "   ";
  if (include_metadata) {
    char metadata_line[MAX_META_LINE_SIZE];
    std::stringstream all_stream;
    auto cores = core_affinity();
    auto cores_size = cores.size();
    if (cores_size > 0) {
      all_stream << ", \"core_affinity\": [";
      for (int i = 0; i < cores_size; ++i) {
        all_stream << cores[i];
        if (i < cores_size - 1) all_stream << ",";
      }
      all_stream << "]";
    }
    bool has_meta = false;
    std::stringstream meta_stream;
    auto meta_size = metadata->size();
    int i = 0;
    for (auto item : *metadata) {
      has_meta = true;
      if (item.second.type() == typeid(unsigned int)) {
        meta_stream << "\"" << item.first << "\":" << std::any_cast<unsigned int>(item.second);
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(int)) {
        meta_stream << "\"" << item.first << "\":" << std::any_cast<int>(item.second);
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(const char *)) {
        meta_stream << "\"" << item.first << "\":\"" << std::any_cast<const char *>(item.second) << "\"";
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(std::string)) {
        meta_stream << "\"" << item.first << "\":\"" << std::any_cast<std::string>(item.second) << "\"";
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(size_t)) {
        meta_stream << "\"" << item.first << "\":\"" << std::any_cast<size_t>(item.second) << "\"";
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(long)) {
        meta_stream << "\"" << item.first << "\":\"" << std::any_cast<long>(item.second) << "\"";
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(ssize_t)) {
        meta_stream << "\"" << item.first << "\":\"" << std::any_cast<ssize_t>(item.second) << "\"";
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(off_t)) {
        meta_stream << "\"" << item.first << "\":\"" << std::any_cast<off_t>(item.second) << "\"";
        if (i < meta_size - 1) meta_stream << ",";
      } else if (item.second.type() == typeid(off64_t)) {
        meta_stream << "\"" << item.first << "\":\"" << std::any_cast<off64_t>(item.second) << "\"";
        if (i < meta_size - 1) meta_stream << ",";
      } else {
        DLIO_PROFILER_LOGINFO("No conversion for type %s", item.first);
      }
      i++;
    }
    if (has_meta) {
      all_stream << "," << meta_stream.str();
    }
    sprintf(metadata_line, R"("hostname":"%s"%s)", this->hostname, all_stream.str().c_str());
    *size = snprintf(data, MAX_LINE_SIZE, "%s{\"id\":\"%d\",\"name\":\"%s\",\"cat\":\"%s\",\"pid\":\"%lu\","
                                          "\"tid\":\"%lu\",\"ts\":\"%llu\",\"dur\":\"%llu\",\"ph\":\"X\",\"args\":{%s}}\n",
                     is_first_char.c_str(), index.load(), event_name, category,
                     process_id, thread_id, start_time, duration, metadata_line);
  } else {
    *size = snprintf(data, MAX_LINE_SIZE, "%s{\"id\":\"%d\",\"name\":\"%s\",\"cat\":\"%s\",\"pid\":\"%lu\","
                                          "\"tid\":\"%lu\",\"ts\":\"%llu\",\"dur\":\"%llu\",\"ph\":\"X\",\"args\":{}}\n",
                     is_first_char.c_str(), index.load(), event_name, category,
                     process_id, thread_id, start_time, duration);
  }
  index++;
}
int dlio_profiler::ChromeWriter::merge_buffer(const char *data, int size) {
  DLIO_PROFILER_LOGDEBUG("ChromeWriter.merge_buffer","");
  uv_rwlock_wrlock(&numlock);
  memcpy(write_buffer + write_size, data, size);
  write_size += size;
  if (write_size >= WRITE_BUFFER_SIZE) {
    write_buffer_op();
    write_size = 0;
  }
  uv_rwlock_wrunlock(&numlock);
  return size;
}