function data = UdpTest(~)

host = '127.0.0.1';
% host = '169.254.1.1';
% host = '10.0.0.200';
% host = '192.168.1.109';
port = 5000;
timeout = 5;
packetLength = 500;
plotLength = 500;

import java.io.*
import java.net.DatagramSocket
import java.net.DatagramPacket
import java.net.InetAddress

%% Setup socket
% Create socket
socket = DatagramSocket;
socket.setReuseAddress(1);
socket.setSoTimeout(timeout);
socket.setReceiveBufferSize(10000);

%% Setup udp packet
% Create Rx & Tx packet
address = InetAddress.getByName(host);
udpPacketTX = DatagramPacket(0, 1, address, port);
udpPacketRX = DatagramPacket(zeros(1, packetLength,'uint8'), packetLength);

ts = rem(now,1)*24*60*60;
timestamp = time;
timestamp.seconds = floor(ts);
timestamp.fraction = floor(rem(ts,1)*1e6);
ctrl = control;
ctrl.time = timestamp;
ctrl.data(1) = 1500;
ctrl.data(2) = 1000;
ctrl.data(3) = 2000;
ctrl.data(4) = 0;
ctrl.data(5) = 0;
ctrl.data(6) = 0;
ctrl.data(7) = 0;
ctrl.data(8) = 10;
content1 = cell2mat(timestamp.ToCell);
content2 = cell2mat(ctrl.ToCell);

idTimestamp = [hex2dec('0') hex2dec('1')];
idControl = [hex2dec('2') hex2dec('1A')];
identifier = hex2dec('DB');

message(1) = iscom();
message(2) = iscom();
message(1).SetIdentifier(idTimestamp);
message(2).SetIdentifier(idControl);
message(1).SetContent(content1);
message(2).SetContent(content2);

payload = uint8([]);
payload = [payload message(1).Pack()];
payload = [payload message(2).Pack()];

messageTX = slink();
messageRX = slink();
msgRX = iscom();
data = struct('gyro',[],'accel',[],'magnet',[]);

%% Setup figure and plot
% Create figure
fig = figure('Name', 'UDP Data');
set(gcf,'NumberTitle','off');
set(gcf, 'Renderer', 'opengl');
set(gcf, 'RendererMode', 'manual');
if isprop(gca,'GraphicsSmoothing')
    set(gcf, 'GraphicsSmoothing', 'off');
end
set(gca, 'SortMethod', 'depth');
hold on; grid on;

% Init memory
bufferRX = uint8([]);
dataAccel = zeros(plotLength,3);

% Create graphics handles
accelHandle = plot(dataAccel);

%% Setup timer
t = timer('Period', 0.05 ,'ExecutionMode', 'fixedRate');
set(t, 'StartFcn', @initTimer);
set(t, 'TimerFcn', @timerCallback);
start(t);

tic
count = 0;
for ii=1:10000
    messageTX.InitMessage();
    messageTX.Identifier = identifier;
    messageTX.LoadPayload(payload);
    messageTX.EndMessage();
    try,
        udpPacketTX.setData(messageTX.Packet);
        socket.send(udpPacketTX);
    end
    try,
        socket.receive(udpPacketRX);
        mssg = udpPacketRX.getData; mssg = typecast(mssg(1:udpPacketRX.getLength), 'uint8');
        bufferRX = [bufferRX; mssg];
    end
    while(numel(bufferRX))
        [result, bufferRX] = messageRX.ReceiveMessage(bufferRX);
        if result == 1
            payload = messageRX.Payload;
            while(numel(payload))
                payload = msgRX.UnPack(payload);
                switch msgRX.IdMajor
                    case 0,
                    case 1,
                        switch msgRX.IdMinor
                            case 26,
                                data.gyro(end+1).time = time(msgRX.Content(1:8));
                                data.gyro(end).data = typecast(msgRX.Content(9:end), 'uint32');
                            case 27,
                                data.accel(end+1).time = time(msgRX.Content(1:8));
                                data.accel(end).data = typecast(msgRX.Content(9:end), 'uint32');
                            case 28,
                                data.magnet(end+1).time = time(msgRX.Content(1:8));
                                data.magnet(end).data = typecast(msgRX.Content(9:end), 'uint32');
                        end
                end
            end
            count = count+1;
        end
    end
    if t.UserData
        % plot accel
        try,
            if numel(data.accel)
                tmp = cell2mat({data.accel(max(end-plotLength+1,1):end).data}');
                dataAccel(end+1-size(tmp,1):end,:) = tmp;
                set(accelHandle(1), 'YData', dataAccel(:,1));
                set(accelHandle(2), 'YData', dataAccel(:,2));
                set(accelHandle(3), 'YData', dataAccel(:,3));
            end
        end
        t.UserData = 0;
    end
    if ~ishandle(fig)
        break;
    end
end
toc
disp(count)

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