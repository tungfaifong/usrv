-- Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

function Start()
	logger.debug("call lua Start")
	local id = timer.CreateTimer(1000, function() logger.debug("call lua timer") end)
	return true
end

function Update(interval)
	logger.debug("call lua Update interval:" .. interval)
	return false
end

function Stop()
	logger.debug("call lua Stop")
end

function OnRecv(net_id, data)
	logger.debug("OnRecv net_id:" .. net_id .. " data:" .. data)
	server.Send(net_id, data, string.len(data))
end
