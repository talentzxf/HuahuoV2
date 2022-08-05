package online.huahuo.backend.security;

import io.jsonwebtoken.Claims;
import io.jsonwebtoken.JwtBuilder;
import io.jsonwebtoken.Jwts;
import io.jsonwebtoken.SignatureAlgorithm;
import io.jsonwebtoken.security.Keys;
import lombok.RequiredArgsConstructor;
import online.huahuo.backend.db.UserDB;
import org.springframework.beans.factory.InitializingBean;
import org.springframework.stereotype.Component;

import javax.crypto.SecretKey;
import javax.crypto.spec.SecretKeySpec;
import java.io.File;
import java.io.FileOutputStream;
import java.io.Serializable;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.time.Duration;
import java.time.Instant;
import java.util.Collections;
import java.util.Date;
import java.util.Map;
import java.util.function.Function;

@Component
@RequiredArgsConstructor
public class JwtTokenUtil implements Serializable, InitializingBean {

    private final SecurityProperties properties;
    private static final long serialVersionUID = 5238463920221750041L;

    private static SecretKey key;
    private static SignatureAlgorithm algorithm;

    static{
        algorithm = SignatureAlgorithm.HS512;
    }

    public String getUsernameFromToken(String token) {
        return getClaimFromToken(token, Claims::getSubject);
    }

    public <T> T getClaimFromToken(String token, Function<Claims, T> claimsResolver) {
        final Claims claims = getAllClaimsFromToken(token);
        return claimsResolver.apply(claims);
    }

    //retrieve expiration date from jwt token
    public Date getExpirationDateFromToken(String token) {
        return getClaimFromToken(token, Claims::getExpiration);
    }

    //for retrieveing any information from token we will need the secret key
    private Claims getAllClaimsFromToken(String token) {
        return Jwts.parser().setSigningKey(key).parseClaimsJws(token).getBody();
    }

    //check if the token has expired
    private Boolean isTokenExpired(String token) {
        final Date expiration = getExpirationDateFromToken(token);
        return expiration.before(new Date());
    }

    private String issueToken(String issuer, Duration expiration, String subject, Map<String, String> claims){
        JwtBuilder jwtBuilder = Jwts.builder()
                .setIssuer(issuer)
                .setIssuedAt(Date.from(Instant.now()))
                .setExpiration(Date.from(Instant.now().plus(expiration)))
                .setSubject(subject);
        for(Map.Entry<String, String> entry: claims.entrySet()){
            jwtBuilder.claim(entry.getKey(), entry.getValue());
        }

        String token = jwtBuilder
                .signWith(key, algorithm)
                .compact();

        return token;
    }

    //generate token for user
    public String generateToken(UserDB userDetails) {
        return issueToken(properties.getTokenIssuer(),
                properties.getTokenExpiration(),
                String.valueOf(userDetails.getUsername()),
                Collections.singletonMap("username1", "password1")
        );
    }

    @Override
    public void afterPropertiesSet() throws Exception {
        File keyFile = new File(properties.getKeyFile());
        if(keyFile.exists()){
            byte[] data = Files.readAllBytes(Paths.get(properties.getKeyFile()));

            key = new SecretKeySpec(data, algorithm.getJcaName());
        }else{
            key = Keys.secretKeyFor(algorithm);

            byte[] keyData = key.getEncoded();

            try(FileOutputStream outputStream = new FileOutputStream(keyFile)){
                outputStream.write(keyData);
            }
        }
    }

//    //while creating the token -
//    //1. Define  claims of the token, like Issuer, Expiration, Subject, and the ID
//    //2. Sign the JWT using the HS512 algorithm and secret key.
//    //3. According to JWS Compact Serialization(https://tools.ietf.org/html/draft-ietf-jose-json-web-signature-41#section-3.1)
//    //   compaction of the JWT to a URL-safe string
//    private String doGenerateToken(Map<String, Object> claims, String subject) {
//
//        return Jwts.builder().setClaims(claims).setSubject(subject).setIssuedAt(new Date(System.currentTimeMillis()))
//                .setExpiration(new Date(System.currentTimeMillis() + JWT_TOKEN_VALIDITY * 1000))
//                .signWith(SignatureAlgorithm.HS512, secret).compact();
//    }
//
//    //validate token
//    public Boolean validateToken(String token, UserDetails userDetails) {
//        final String username = getUsernameFromToken(token);
//        return (username.equals(userDetails.getUsername()) && !isTokenExpired(token));
//    }
}
