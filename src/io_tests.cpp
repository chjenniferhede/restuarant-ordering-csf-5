#include <stdexcept>
#include <cstring>
#include <cassert>
#include "io.h"
#include "csapp.h"
#include "except.h"
#include "tctest.h"

// ----------------------------------------------------------------------
// The IO::send and IO::receive functions operate on a file descriptor,
// so the tests can use the TmpFile class to write to and read from a
// temporary file, in order to test the behavior of IO::send and
// IO::receive.
// ----------------------------------------------------------------------

struct TmpFile {
private:
  char filename[20];
  bool created; // if the temp file was created successfully
  int writefd, readfd;

public:
  TmpFile();
  TmpFile(const std::string &filename);
  ~TmpFile();

  void reopen_as_readonly();
  int get_write_fd() const;
  void write_string(const std::string &s);
  void done_writing();
  int get_read_fd() const;
  std::string read_string();
};

TmpFile::TmpFile()
  : created(false)
  , writefd(-1)
  , readfd(-1) {
  std::strcpy(filename, "iotestXXXXXX");
  writefd = mkstemp(filename);
  if (writefd < 0)
    throw std::runtime_error("could not open temp file");
  created = true;
}

// This constructor can be used to open a named file.
// Useful for opening /dev/null to test whether IO::receive
// throws IOException when it fails to read a message.
TmpFile::TmpFile(const std::string &fname)
  : created(false)
  , writefd(-1)
  , readfd(-1) {
  assert(fname.size() < sizeof(filename));
  std::strcpy(filename, fname.c_str());
  writefd = open(filename, O_WRONLY);
  if (writefd < 0)
    throw std::runtime_error("could not open '" + fname + "' for writing");
}

TmpFile::~TmpFile() {
  if (writefd >= 0)
    close(writefd);
  if (readfd >= 0)
    close(readfd);
  if (created)
    unlink(filename);
}

// Reopen the write file descriptor as read-only,
// as a way of testing whether IO::send throws an IOException
// if it can't write to the file descriptor.
void TmpFile::reopen_as_readonly() {
  assert(writefd >= 0);
  close(writefd);
  writefd = open(filename, O_RDONLY);
  if (writefd < 0)
    throw std::runtime_error("couldn't reopen temp file as read-only?");
}

int TmpFile::get_write_fd() const {
  assert(writefd >= 0);
  return writefd;
}

void TmpFile::write_string(const std::string &s) {
  assert(writefd >= 0);
  if (rio_writen(writefd, s.data(), s.size()) != ssize_t(s.size()))
    throw std::runtime_error("couldn't write to temp file");
}

void TmpFile::done_writing() {
  assert(writefd >= 0);
  close(writefd);
  writefd = -1;

  readfd = open(filename, O_RDONLY);
  if (readfd < 0)
    throw std::runtime_error("could not re-open temp file for reading");
}

int TmpFile::get_read_fd() const {
  assert(readfd >= 0);
  return readfd;
}

std::string TmpFile::read_string() {
  assert(readfd >= 0);
  char buf[1024];
  ssize_t n;
  std::string s;
  while ((n = read(readfd, buf, 1024)) > 0) {
    s.append(buf, buf + n);
  }
  if (n < 0)
    throw std::runtime_error("error reading from temp file");
  return s;
}

struct TestObjs {

};

// ----------------------------------------------------------------------
// Unit test code starts here
// ----------------------------------------------------------------------

TestObjs *setup();
void cleanup(TestObjs *objs);

void test_send(TestObjs *objs);
void test_send_multiple(TestObjs *objs);
void test_receive_1(TestObjs *objs);
void test_receive_2(TestObjs *objs);
void test_send_ioerror(TestObjs *objs);
void test_receive_ioerror_1(TestObjs *objs);
void test_receive_ioerror_2(TestObjs *objs);
void test_receive_ioerror_3(TestObjs *objs);

int main(int argc, char **argv) {
  if (argc > 1)
    tctest_testname_to_execute = argv[1];

  TEST_INIT();

  TEST(test_send);
  TEST(test_send_multiple);
  TEST(test_receive_1);
  TEST(test_receive_2);
  TEST(test_send_ioerror);
  TEST(test_receive_ioerror_1);
  TEST(test_receive_ioerror_2);
  TEST(test_receive_ioerror_3);

  TEST_FINI();
}

TestObjs *setup() {
  return new TestObjs();
}

void cleanup(TestObjs *objs) {
  delete objs;
}

void test_send(TestObjs *objs) {
  TmpFile tf;

  IO::send("hello, world!", tf.get_write_fd());
  tf.done_writing();
  std::string s = tf.read_string();

  ASSERT(s == "0014hello, world!\n");
}

void test_send_multiple(TestObjs *objs) {
  TmpFile tf;

  IO::send("first message", tf.get_write_fd());
  IO::send("second message", tf.get_write_fd());
  IO::send("third message", tf.get_write_fd());
  tf.done_writing();

  std::string s = tf.read_string();

  ASSERT(s == "0014first message\n0015second message\n0014third message\n");
}

void test_receive_1(TestObjs *objs) {
  TmpFile tf;

  tf.write_string("0014hello, world!\n");
  tf.done_writing();
  std::string s;
  IO::receive(tf.get_read_fd(), s);

  ASSERT(s == "hello, world!");
}

void test_receive_2(TestObjs *objs) {
  // Make sure that exactly 4 digits are treated as the
  // length prefix, and no more
  TmpFile tf;

  tf.write_string("00021\n0011ABCDEFGHIJ\n");
  tf.done_writing();

  std::string s;
  IO::receive(tf.get_read_fd(), s);
  ASSERT(s == "1");

  std::string s2;
  IO::receive(tf.get_read_fd(), s2);
  ASSERT(s2 == "ABCDEFGHIJ");
}

void test_send_ioerror(TestObjs *objs) {
  TmpFile tf;

  tf.reopen_as_readonly();
  try {
    IO::send("hello, world!", tf.get_write_fd());
    FAIL("send to readonly file should throw IOException");
  } catch (IOException &ex) {
    // good
  }
}

void test_receive_ioerror_1(TestObjs *objs) {
  // Test that IO::receive throws IOException if the
  // length prefix is not valid
  TmpFile tf;

  tf.write_string("002a\n");
  tf.done_writing();
  try {
    std::string s;
    IO::receive(tf.get_read_fd(), s);
    FAIL("reading malformed encoded string should throw IOException");
  } catch (IOException &ex) {
    // good
  }
}

void test_receive_ioerror_2(TestObjs *objs) {
  // Test that IO::receive throws IOException if the
  // terminating newline is not present
  TmpFile tf;

  tf.write_string("0005abcd0004efgh\n");
  tf.done_writing();
  try {
    std::string s;
    IO::receive(tf.get_read_fd(), s);
    FAIL("reading encoded string w/o terminating newline should throw IOException");
  } catch (IOException &ex) {
    // good
  }
}

void test_receive_ioerror_3(TestObjs *objs) {
  TmpFile tf("/dev/null");
  tf.done_writing();

  try {
    // when we try to read from /dev/null, we should get
    // an immediate EOF
    std::string s;
    IO::receive(tf.get_read_fd(), s);
    FAIL("IO::receive reading from /dev/null should throw IOException");
  } catch (IOException &ex) {
    // good
  }
}
