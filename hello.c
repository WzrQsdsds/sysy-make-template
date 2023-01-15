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

lhs || rhs

int result = 1;
if (lhs == 0) {
  result = rhs != 0;
}
lhs && rhs

int result = 0;
if (lhs != 0) {
  result = rhs != 0;
}

  // if 的条件判断部分
  %0 = load @a
  br %0, %then, %else

// if 语句的 if 分支
%then:
  %1 = load @a
  %2 = add %1, 1
  store %2, @a
  jump %end

// if 语句的 else 分支
%else:
  store 0, @a
  jump %end