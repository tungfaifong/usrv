-- Copyright (c) 2019-2020 TungFai Fong 

function Start()
	print("call lua Start")
	return true
end

function Update(interval)
	print("call lua Update interval:" .. interval)
end

function Stop()
	print("call lua Stop")
end

function OnRecv(net_id, data)
	print("OnRecv net_id:" .. net_id .. " data:" .. data)
end