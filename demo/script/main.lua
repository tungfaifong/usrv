-- Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

function Start()
	logger.info("call lua Start")
	local id = timer.CreateTimer(1000, function() logger.info("call lua timer") end)
	return true
end

function Update(interval)
	logger.info("call lua Update interval:" .. interval)
end

function Stop()
	logger.info("call lua Stop")
end

function OnRecv(net_id, data)
	logger.info("OnRecv net_id:" .. net_id .. " data:" .. data)
	server.Send(net_id, data, string.len(data))
end
