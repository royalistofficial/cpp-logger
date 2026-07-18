#include "TestFramework.hpp"

#include "logger/FileLogSink.hpp"

#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>

using logger::FileLogSink;
using logger::FlushPolicy;

namespace {

/// Временный файл, удаляющийся вместе с объектом.
class TempFile {
public:
    explicit TempFile(std::string name)
        : path_("/tmp/logger_test_" + std::move(name)) {
        std::remove(path_.c_str());
    }

    ~TempFile() { std::remove(path_.c_str()); }

    TempFile(const TempFile&) = delete;
    TempFile& operator=(const TempFile&) = delete;

    const std::string& path() const { return path_; }

    std::string read() const {
        std::ifstream input(path_);
        std::ostringstream buffer;
        buffer << input.rdbuf();
        return buffer.str();
    }

private:
    std::string path_;
};

}  // namespace

TEST(ЗаписьПопадаетВФайл) {
    const TempFile file("write.log");
    {
        FileLogSink sink(file.path());
        sink.write("первая\n");
        sink.write("вторая\n");
    }
    CHECK_EQ(file.read(), std::string("первая\nвторая\n"));
}

TEST(ФайлОткрываетсяНаДописывание) {
    const TempFile file("append.log");
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

TEST(СбросБуфераДелаетЗаписьВидимойСразу) {
    const TempFile file("flush.log");
    FileLogSink sink(file.path(), FlushPolicy::EveryRecord);
    sink.write("сразу\n");

    // Файл ещё не закрыт, но данные уже должны быть на диске.
    CHECK_EQ(file.read(), std::string("сразу\n"));
}

TEST(ПустоеИмяФайлаОтвергается) {
    CHECK_THROWS_AS(FileLogSink(""), std::runtime_error);
}

TEST(НесуществующийКаталогДаётОшибкуСПояснением) {
    bool thrown = false;
    try {
        FileLogSink sink("/no/such/directory/app.log");
    } catch (const std::runtime_error& error) {
        thrown = true;
        const std::string message = error.what();
        CHECK(message.find("/no/such/directory/app.log") != std::string::npos);
        // Пояснение от системы должно быть, а не только имя файла.
        CHECK(message.size() > std::string("/no/such/directory/app.log").size());
    }
    CHECK(thrown);
}

TEST(ПутьДоступенЧерезГеттер) {
    const TempFile file("path.log");
    const FileLogSink sink(file.path());
    CHECK_EQ(sink.path(), file.path());
}
