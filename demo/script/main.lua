-- Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

function Start()
	logger.log(0, "call lua Start")
	local id = timer.CreateTimer(1000, function() logger.log(0, "call lua timer") end)
	return true
end

function Update(interval)
	logger.log(0, "call lua Update interval:" .. interval)
end

function Stop()
	logger.log(0, "call lua Stop")
end

function OnRecv(net_id, data)
	logger.log(0, "OnRecv net_id:" .. net_id .. " data:" .. data)
	server.Send(net_id, data, string.len(data))
end
