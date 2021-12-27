# ON ZEUS: if you don't have python3 installed, you need to run 
# 
#    zeus$   module add python/3.5.0
# 
# (You might as well add that to your ~/.bashrc file).


import subprocess

progname = "./rec5"

def ensure_built():
    makey = subprocess.getoutput("make")
    print("calling make... ", end="", flush=True)
    if "Error" in makey:
        print(makey)
        print("\n\nfailed to make; not running any tests.")
        exit()
    print("\tcode built.")

def call_it(*args):
    return subprocess.getoutput(progname + " " + " ".join(args))

def score(ans,*args):
    got = call_it(*args)
    if ans==got:
        print(".",end="",flush=True)
        return 1
    print("\nERROR:\t %s %s\n\texpected: %s\n\tgot:      %s" % (progname, " ".join(args),ans,got))
    return 0

def main():
    ensure_built()
    
    # each test has the expected answer first, then
    # the exact arguments we'd pass to ./rec5.
    tests = [
        ("100","mul_20","5"),
        ("120","mul_20","6"),
        ("-20","mul_20","-1"),
        ("2000","mul_20","100"),
        
        (  "6","add3",'1','2','3'),
        ( "30","add3",'10','10','10'),
        ("600","add3",'100','200','300'),
        (  "2","add3",'1','-2','3'),
        
        (  "5", "max2",  '5',  '4'),
        ( "20", "max2", '10', '20'),
        ( "-4", "max2", '-4', '-8'),
        ( "10", "max2", '10', '10'),
        
        ( "15", "sumUpTo",  '5'),
        ( "45", "sumUpTo",  '9'),
        (  "1", "sumUpTo",  '1'),
        (  "0", "sumUpTo",  '0'),
        (  "0", "sumUpTo", '-5'),
        
        (   '1', "collatzLength", '1'),
        (   '3', "collatzLength", '4'),
        (   '6', "collatzLength", '5'),
        (  '17', "collatzLength", '7'),
        (  '12', "collatzLength", '2048'),
        (   '1', "collatzLength", '1'),
        
        ( '-1', "fact", '-1'),
        ( '-1', "fact", '-1'),
        ( '1', "fact", '1'),
        ( '24', "fact", '4'),
        ( '120', "fact", '5'),
        ( '3628800', "fact", '10'),
        ( '121645100408832000', "fact", '19'),
        
        ]    
    
    tally = 0
    for test in tests:
        tally += score(*test)
    
    print("\n\nscore: %s/%s" % (tally,len(tests)))

if __name__=="__main__":
    main()
