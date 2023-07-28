import sys

f = open(sys.argv[1], 'rb')
d = {b'public': b'C:\\Users\\Public'}

for line in f.readlines():
    line = line.replace(b'^', b'')
    if line.find(b'&cls\n') != -1:
        print('cls')
        continue
    
    while True:
        n1 = line.find(b'%')
        if n1 == -1:
            break
        n2 = line[n1+1:].find(b'%')
        
        var = line[n1:n1+n2+2]
        if var.find(b':~') == -1:
            if var.lower() == b'%errorlevel%':
                line = line.replace(var, b'$errorlevel$', 1)
            line = line.replace(var, b'', 1)
        else:
            varsplit = var[1:-1].split(b':~')
            varname = varsplit[0].lower()
            varslice = [int(x) for x in varsplit[1].split(b',')]
            
            if varname in d:
                newstr = d[varname][varslice[0]:varslice[0]+varslice[1]]
            else:
                print('Error!')
                newstr = b''
            
            line = line.replace(var, newstr, 1)
    
    if line.find(b'@set') != -1:
        n1 = line.find(b'"')
        n2 = line[n1+1:].find(b'"')
        setarg = line[n1+1:n1+n2+1].split(b'=', 1)
        d[setarg[0].lower()] = setarg[1]

    print(str(line, 'utf-8').lstrip().replace('$errorlevel$', '%ErrorLevel%'), end='')