package online.huahuo.backend.db;

import lombok.RequiredArgsConstructor;
import lombok.extern.slf4j.Slf4j;
import org.apache.commons.lang3.RandomStringUtils;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.security.authentication.AuthenticationProvider;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Service;
import org.springframework.transaction.annotation.Transactional;

import java.util.Date;

@Service
@RequiredArgsConstructor
@Slf4j
public class UserService implements InitializingBean {
    private final UserRepository userRepository;
    private final AuthenticationProvider authenticationProvider;
    private final PasswordEncoder passwordEncoder;

    @Value("${huahuo.jwt.createDefaultUser}")
    private boolean createDefaultUser;
    @Value("${huahuo.jwt.defaultUser.username}")
    private String defaultUserName;
    @Value("${huahuo.jwt.passwordStrength}")
    private int passwordStrength;

    public UserDB createUser(String username, String pwd, UserRole role){
        UserDB user = UserDB.builder()
                .username(username)
                .password(passwordEncoder.encode(pwd))
                .nickname(username)
                .role(role)
                .status(UserStatus.ACTIVE)
                .createTime(new Date())
                .modifiedTime(new Date())
                .lastLoginTime(new Date())
                .build();

        return createUser(user, false, pwd);
    }

    public UserDB createUser(UserDB user, String clearTextPassword) {
        return createUser(user, false, clearTextPassword);
    }

    @Transactional
    public UserDB createUser(UserDB user, boolean loginAfterCreate, String clearTextpassword) {
        UserDB save = userRepository.save(user);
        try {
            if (loginAfterCreate) {
                UsernamePasswordAuthenticationToken credentials = new UsernamePasswordAuthenticationToken(save.getUsername(), clearTextpassword);
                SecurityContextHolder.getContext().setAuthentication(authenticationProvider.authenticate(credentials));
            }
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
        return save;
    }


    public void createDefaultUser() {
        if (createDefaultUser) {
            log.info("Creating default user. This should only be used in test environments.");
            log.info("Please set property: huahuo.jwt.createDefaultUser = false in official environments.");

            if (null == userRepository.findByUsername(defaultUserName)) { // If the default user doesn't exist. Create it

                String defaultUserPassword = RandomStringUtils.random(passwordStrength, true, true);

                log.info("Begin to create default username:" + defaultUserName + " password:" + defaultUserPassword);

                UserDB user = UserDB.builder()
                        .username(defaultUserName)
                        .password(passwordEncoder.encode(defaultUserPassword))
                        .nickname(defaultUserName)
                        .role(UserRole.ADMIN)
                        .status(UserStatus.ACTIVE)
                        .createTime(new Date())
                        .modifiedTime(new Date())
                        .lastLoginTime(new Date())
                        .build();

                this.createUser(user, true, defaultUserPassword);

                log.info("Default admin user created with password:{}", defaultUserPassword);
            } else {
                log.info("Default user already exists, no need to create it");
            }
        }
    }


    @Override
    public void afterPropertiesSet() throws Exception {
        createDefaultUser();
    }
}
