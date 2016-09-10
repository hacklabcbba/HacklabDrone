function data = UdpPlot(ip, port, timeout, numSample)
if ~exist('ip'), ip = '127.0.0.1'; end
if ~exist('port'), port = 5000; end
if ~exist('timeout'), timeout = 10; end
if ~exist('numSample'), numSample = 500; end

import java.io.*
import java.net.DatagramSocket
import java.net.DatagramPacket
import java.net.InetAddress

%% Setup socket
socket = DatagramSocket;
socket.setReuseAddress(1);
socket.setSoTimeout(timeout);
socket.setReceiveBufferSize(10000);

%% Setup udp packet
packetLength = 500;
address = InetAddress.getByName(ip);
udpPacketTX = DatagramPacket(0, 1, address, port);
udpPacketRX = DatagramPacket(zeros(1, packetLength, 'uint8'), packetLength);

%% Assign memory
buffer = uint8([]);
data = struct('gyro', [], 'accel', [], 'magnet', []);

%% Setup figure and plot
% Create figure
fig = figure('Name', 'UDP Data');
set(gcf, 'NumberTitle', 'off');
set(gcf, 'Renderer', 'opengl');
set(gcf, 'RendererMode', 'manual');
if isprop(gca,'GraphicsSmoothing')
    set(gcf, 'GraphicsSmoothing', 'off');
end
set(gca, 'SortMethod', 'depth');
hold on; grid on;

% Create graphics handles
accelHandle = plot(zeros(numSample, 3));

%% Setup timer
t = timer('Period', 0.05, 'ExecutionMode', 'fixedRate');
set(t, 'StartFcn', @initTimer);
set(t, 'TimerFcn', @timerCallback);
start(t);

%% Setup slink & iscom message
message = iscom();
slinkMessageRX = slink();
slinkMessageTX = slink();
slinkMessageTX.InitMessage();
slinkMessageTX.Identifier = 0;
slinkMessageTX.EndMessage();

while true
    try
        socket.receive(udpPacketRX);
        mssg = udpPacketRX.getData; mssg = typecast(mssg(1:udpPacketRX.getLength), 'uint8');
        buffer = [buffer; mssg];
    end
    while (numel(buffer))
        [result, buffer] = slinkMessageRX.ReceiveMessage(buffer);
        if result == 1
            payload = slinkMessageRX.Payload;
            while (numel(payload))
                payload = message.UnPack(payload);
                switch message.IdMajor
                    case 0,
                    case 1,
                        switch message.IdMinor
                            case 26,
                                data.gyro(end + 1).time = timeConvert(typecast(message.Content(1:8), 'uint32'));
                                data.gyro(end).data = typecast(message.Content(9:end), 'uint32');
                            case 27,
                                data.accel(end + 1).time = timeConvert(typecast(message.Content(1:8), 'uint32'));
                                data.accel(end).data = typecast(message.Content(9:end), 'uint32');
                            case 28,
                                data.magnet(end + 1).time = timeConvert(typecast(message.Content(1:8), 'uint32'));
                                data.magnet(end).data = typecast(message.Content(9:end), 'uint32');
                        end
                end
            end
        end
    end
    if t.UserData
        % Send slink message
        try
            udpPacketTX.setData(slinkMessageTX.Packet);
            socket.send(udpPacketTX);
        end
        
        % Plot accel
        try
            if numel(data.accel)
                tmp = data.accel(max(end - numSample + 1, 1):end);
                tmpData = padarray(cell2mat({tmp.data}'), [numSample - numel(tmp), 0], 'replicate', 'pre');
                set(accelHandle(1), 'YData', tmpData(:, 1));
                set(accelHandle(2), 'YData', tmpData(:, 2));
                set(accelHandle(3), 'YData', tmpData(:, 3));
            end
        end
        t.UserData = 0;
    end
    if ~ishandle(fig)
        break;
    end
end

stop(t);
delete(t);
socket.close;
end

function initTimer(src, event)
src.UserData = 0;
end

function timerCallback(src, event)
src.UserData = 1;
drawnow;
end

function time = timeConvert(arg)
arg = double(arg);
time = arg(1) + arg(2) * 1e-6;
end
