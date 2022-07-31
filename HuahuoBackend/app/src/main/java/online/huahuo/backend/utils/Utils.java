package online.huahuo.backend.utils;

import java.nio.charset.StandardCharsets;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;

public class Utils {
    public static String bytesToHex(byte[] bytes) {
        StringBuilder sb = new StringBuilder();
        for (byte b : bytes) {
            sb.append(String.format("%02x", b));
        }
        return sb.toString();
    }

    public static String hashBytes(byte[] inputs) throws NoSuchAlgorithmException {
        MessageDigest md = MessageDigest.getInstance("SHA3-256");
        byte[] checksum = md.digest(inputs);
        return bytesToHex(checksum);
    }

    public static String hashString(String input) throws NoSuchAlgorithmException {
        return hashBytes(input.getBytes(StandardCharsets.UTF_8));
    }
}
