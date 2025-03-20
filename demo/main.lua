-- Copyright (c) 2022 TungFai Fong <iam@tungfaifong.com>

function reverse_string(s)
	local reversed = {}
	for i = #s, 1, -1 do
		reversed[#reversed + 1] = s:sub(i, i)
	end
	return table.concat(reversed)
end

function Start()
	-- logger.debug("call lua Start")
	-- local id = timer.CreateTimer(1000, function() logger.debug("call lua timer") end)
	return true
end

function Update(interval)
	return false
end

function Stop()
	-- logger.debug("call lua Stop")
end

function OnConn(net_id, ip, port)
	-- logger.log(1, 1, "OnConn net_id:" .. net_id.pid .. " ip:" .. ip .. " port:" .. port)
end

function OnRecv(net_id, data)
	-- logger.log(1, 1, "OnRecv net_id:" .. net_id.pid .. " data:" .. data)
	msg = data -- reverse_string(data)
	server.Send(net_id, msg)
end

function OnDisc(net_id)
	-- logger.log(1, 1, "OnDisc net_id:" .. net_id.pid)
end
