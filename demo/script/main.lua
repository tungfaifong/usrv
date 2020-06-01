-- Copyright (c) 2019-2020 TungFai Fong 

function Start()
    print("call lua Start")
    return true
end

function Update(interval)
    print("call lua Update interval:" .. interval)
    local cpp_class = CPPClass()
    cpp_class:print()
end

function Stop()
    print("call lua Stop")
end
