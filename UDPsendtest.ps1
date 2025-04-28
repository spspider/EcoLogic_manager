$udpClient = New-Object System.Net.Sockets.UdpClient
$remoteEndPoint = New-Object System.Net.IPEndPoint([System.Net.IPAddress]::Parse("192.168.1.143"), 4210)
$message = "0:501"
$bytes = [System.Text.Encoding]::ASCII.GetBytes($message)
$udpClient.Send($bytes, $bytes.Length, $remoteEndPoint)
$udpClient.Close()
