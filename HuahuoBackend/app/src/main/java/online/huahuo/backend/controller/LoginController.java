package online.huahuo.backend.controller;

import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.security.Keys;
import io.jsonwebtoken.SignatureAlgorithm;
import lombok.Data;
import lombok.RequiredArgsConstructor;
import online.huahuo.backend.db.UserDB;
import online.huahuo.backend.db.UserRepository;
import online.huahuo.backend.db.UserStatus;
import online.huahuo.backend.exception.UserNotFoundException;
import online.huahuo.backend.security.SecurityProperties;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.security.authentication.AuthenticationProvider;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.transaction.annotation.Transactional;
import org.springframework.web.bind.annotation.PostMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;

import javax.crypto.SecretKey;
import java.time.Instant;
import java.util.Date;

@Data
class LoginStatus{
    private String userName;
    private String failReason;
    private String jwtToken;
    private HttpStatus httpStatus;
}

@RestController
@RequiredArgsConstructor
public class LoginController {
    private final UserRepository userRepository;
    private final AuthenticationProvider authenticationProvider;
    private final SecurityProperties properties;

    @Transactional
    public String issueToken(String username) {
        UserDB user = userRepository.findByUsername(username);
        if (user == null) throw new UserNotFoundException(username);
        Instant now = Instant.now();
        Instant expiry = Instant.now().plus(properties.getTokenExpiration());

        SecretKey key = Keys.secretKeyFor(SignatureAlgorithm.HS512);

        String token = Jwts.builder()
                .setIssuer(properties.getTokenIssuer())
                .setIssuedAt(Date.from(now))
                .setExpiration(Date.from(expiry))
                .setSubject(username)
                .claim("role", user.getRole().name())
                .signWith(key, SignatureAlgorithm.HS512)
                .compact();

        return token;
    }

    @PostMapping("/login")
    ResponseEntity<?> login(@RequestParam(required = false) String username, @RequestParam(required = false) String password){
        LoginStatus loginStatus = new LoginStatus();
        if(username == null || password == null){
            loginStatus.setFailReason("Username or pwd is null");
            loginStatus.setHttpStatus(HttpStatus.BAD_REQUEST);
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        loginStatus.setUserName(username);

        UserDB user = userRepository.findByUsername(username);
        if(user == null){
            loginStatus.setFailReason("Can't find user!");
            loginStatus.setHttpStatus(HttpStatus.BAD_REQUEST);
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        if(user.getStatus() != UserStatus.ACTIVE){
            loginStatus.setFailReason("User is not active!");
            return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
        }

        String token = issueToken(username);

        loginStatus.setJwtToken(token);
        UsernamePasswordAuthenticationToken credentials = new UsernamePasswordAuthenticationToken(username, password);
        SecurityContextHolder.getContext().setAuthentication(authenticationProvider.authenticate(credentials));

        loginStatus.setHttpStatus(HttpStatus.OK);
        return new ResponseEntity<>(loginStatus, loginStatus.getHttpStatus());
    }
}
