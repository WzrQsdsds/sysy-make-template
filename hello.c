int main() {
  int a = 2;
  {
    a = 4;
    int a = 5;
  }
  {
    a = 7;
    int a = 10;
      }
  return a;
}