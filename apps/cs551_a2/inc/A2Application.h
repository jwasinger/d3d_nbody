#ifndef A2APPLICATION_H
#define A2APPLICATION_H 

class A2Application : public Application
{
public:
    A2Application(void);
    ~A2Application();
    bool Init(void);
    
protected:
    virtual void Render(void);
    virtual void Update(void);
};
#endif
