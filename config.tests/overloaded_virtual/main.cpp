
class A
{
public:
    A() {}
    virtual void foo() {}
};

class B : public A
{
public:
    B() {}
    void foo(int) {}
};

int main( int , char ** ) {
    A* a = new B;
    a->foo();
    return 0;
}

