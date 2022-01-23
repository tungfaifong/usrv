-- Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

function Start()
	logger.trace("call lua Start")
	local id = timer.CreateTimer(1000, function() print("call lua timer") end)
	return true
end

function Update(interval)
	logger.trace("call lua Update interval:" .. interval)
end

function Stop()
	logger.trace("call lua Stop")
end

function OnRecv(net_id, data)
	logger.trace("OnRecv net_id:" .. net_id .. " data:" .. data)
	server.Send(net_id, data, string.len(data))
end
