#include "TestFramework.hpp"

#include "logger/FileLogSink.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

using logger::FileLogSink;

namespace {

class TempFile {
public:
    explicit TempFile(std::string name) : path_("/tmp/" + name) {
        std::remove(path_.c_str());
    }

    ~TempFile() { std::remove(path_.c_str()); }

    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;

    const std::string& path() const noexcept { return path_; }

    std::string read() const {
        std::ifstream in(path_);
        std::ostringstream out;
        out << in.rdbuf();
        return out.str();
    }

private:
    std::string path_;
};

}

TEST(ЗаписьПопадаетВФайл) {
    TempFile file("logger_sink_write.log");

    {
        FileLogSink sink(file.path());
        sink.write("первая\n");
        sink.write("вторая\n");
    }

    CHECK_EQ(file.read(), std::string("первая\nвторая\n"));
}

TEST(ПовторноеОткрытиеДописываетВКонец) {
    TempFile file("logger_sink_append.log");

    {
        FileLogSink sink(file.path());
        sink.write("старое\n");
    }
    {
        FileLogSink sink(file.path());
        sink.write("новое\n");
    }

    CHECK_EQ(file.read(), std::string("старое\nновое\n"));
}

TEST(ДанныеВидныСразуБезЗакрытияФайла) {
    TempFile file("logger_sink_flush.log");

    FileLogSink sink(file.path());
    sink.write("до закрытия\n");

    // Файл ещё открыт, но flush в write() уже вытолкнул данные на диск.
    CHECK_EQ(file.read(), std::string("до закрытия\n"));
}

TEST(НедоступныйПутьДаётИсключение) {
    bool thrown = false;
    std::string message;

    try {
        FileLogSink sink("/nonexistent_directory_12345/app.log");
    } catch (const std::runtime_error& error) {
        thrown = true;
        message = error.what();
    }

    CHECK(thrown);
    CHECK(message.find("app.log") != std::string::npos);
}

TEST(ПустоеИмяФайлаДаётИсключение) {
    bool thrown = false;
    try {
        FileLogSink sink("");
    } catch (const std::runtime_error&) {
        thrown = true;
    }

    CHECK(thrown);
}

TEST(ОшибкаЗаписиПослеОткрытияНеПроглатывается) {
    std::ifstream probe("/dev/full");
    if (!probe.good()) {
        return; 
    }
    probe.close();

    FileLogSink sink("/dev/full");

    bool thrown = false;
    try {
        sink.write("эта строка не поместится\n");
    } catch (const std::runtime_error&) {
        thrown = true;
    }

    CHECK(thrown);
}
