int x, y;

int t() {
  x = x + 1;
  return 1;
}

int f() {
  y = y + 1;
  return 0;
}

int main() {
  int sum = 0;
  sum = sum + (f() || f());
  sum = sum + (f() || t());
  sum = sum + (t() || f());
  sum = sum + (t() || t());
  sum = sum + (f() && f());
  sum = sum + (f() && t());
  sum = sum + (t() && f());
  sum = sum + (t() && t());
  return sum;
}