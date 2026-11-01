# Nama program hasil kompilasi
TARGET = main

# Compiler C++
CXX = g++
CXXFLAGS = -Wall -g

# Target utama untuk kompilasi
all:
	$(CXX) $(CXXFLAGS) main.cpp -o $(TARGET) -lsqlite3 -lsodium

# Target untuk menjalankan program langsung setelah compile
run: all
	./$(TARGET)

# Target untuk membersihkan file hasil kompilasi
clean:
	rm -f $(TARGET)
