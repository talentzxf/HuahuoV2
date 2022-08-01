package online.huahuo.backend.security;

import lombok.SneakyThrows;
import lombok.extern.slf4j.Slf4j;
import online.huahuo.backend.exception.WrongPasswordFormatException;
import online.huahuo.backend.utils.Utils;
import org.apache.commons.lang3.RandomStringUtils;
import org.springframework.security.crypto.password.PasswordEncoder;

@Slf4j
public class UserPasswordEncoder implements PasswordEncoder {
    private static final String delimiter = ":";
    private int mStrength = 10;

    public UserPasswordEncoder(int strength){
        mStrength = strength;
    }

    @SneakyThrows
    @Override
    public String encode(CharSequence clearText){
        String salt = RandomStringUtils.random(mStrength, true, true);
        String saltedPassword = clearText + delimiter + salt;
        String hashedPassword = Utils.hashString(saltedPassword);
        return salt + delimiter + hashedPassword;
    }

    @SneakyThrows
    @Override
    public boolean matches(CharSequence clearText, String hashedText){
        String[] splittedHashedText = hashedText.split(delimiter);
        if(splittedHashedText.length != 2 ){
            log.error("Hashed password should contain one and only one {}", delimiter);
            throw new WrongPasswordFormatException();
        }

        String salt = splittedHashedText[0];
        String saltedPassword = clearText + delimiter + salt;

        String hashedPassword = Utils.hashString(saltedPassword);
        return hashedPassword.equals(splittedHashedText[1]);
    }
}
